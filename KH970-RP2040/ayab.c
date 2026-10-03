/* Core 1: TinyUSB CDC + AYAB API v6 protocol translation.
 *
 * This half makes the KH-970 (a CSI machine that the CB-1 computer normally
 * drives) appear to AYAB Desktop as a classic KH-910/950.  The desktop talks
 * to the real AYAB firmware with SLIP-framed API v6 messages; we speak the
 * same protocol and translate:
 *
 *   desktop -> firmware            firmware -> machine        machine -> firmware
 *   -----------------              -------------------        -------------------
 *   cnfLine(line, 25 B)      ->    needle row in pattern[]    (via core 0)
 *   reqStart/reqInit          ->    cnfStart/cnfInit replies
 *   reqInfo                   ->    cnfInfo reply
 *                                                    0xB0     ->   reqLine(next)
 *                                                    0xB1/B2  ->   indState (position)
 *                                                    0xB6-B8  ->   indState (carriage)
 *                                                    config/0x80 -> indState (ready)
 *
 * All TinyUSB calls live on core 1; core 0 never touches USB.
 */
#include <string.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "shared.h"
#include "slip.h"
#include "ayab.h"
#include "dbg.h"

/* ---- AYAB API v6 constants (mirror ayab-firmware/src/ayab/com.h) -------- */
enum {
    AYAB_reqStart = 0x01, AYAB_cnfStart = 0xC1,
    AYAB_reqLine  = 0x82, AYAB_cnfLine  = 0x42,
    AYAB_reqInfo  = 0x03, AYAB_cnfInfo  = 0xC3,
    AYAB_reqTest  = 0x04, AYAB_cnfTest  = 0xC4,
    AYAB_indState = 0x84,
    AYAB_reqInit  = 0x05, AYAB_cnfInit  = 0xC5,
};

#define AYAB_API_VERSION   6
#define AYAB_FW_MAJ        1
#define AYAB_FW_MIN        0
#define AYAB_FW_PATCH      0

/* Error codes (subset of ayab-firmware fsm.h ErrorCode). */
#define AYAB_OK                0x00
#define AYAB_ERR_EXPECTED      0x01
#define AYAB_ERR_CHECKSUM      0x04
#define AYAB_ERR_MACHINE_TYPE  0x10
#define AYAB_ERR_WRONG_STATE   0xEF

/* OpState (indState field 2). */
#define AYAB_STATE_INIT  1
#define AYAB_STATE_READY 2
#define AYAB_STATE_KNIT  3

/* Carriage (indState field 7) and direction (field 9), matching the firmware
 * enums (Knit=0, Lace=1, Garter=2; Left=0, Right=1, None=0xFF). */
#define AYAB_CARRIAGE_KNIT   0
#define AYAB_CARRIAGE_LACE   1
#define AYAB_CARRIAGE_GARTER 2
#define AYAB_CARRIAGE_NONE   0xFF
#define AYAB_DIR_LEFT        0
#define AYAB_DIR_RIGHT       1
#define AYAB_DIR_NONE        0xFF

#define AYAB_LINE_LEN 25   /* 200 needles / 8 bits */

/* The reference firmware inverts each line byte before use because its
 * solenoid hardware is active-low.  For the KH-970 CSI bitmap we pass the
 * data through unchanged (1 = needle selected).  Flip this and reflash if the
 * needle selection comes out inverted on the machine. */
#define AYAB_INVERT_LINE_DATA 0

/* ---- local state (core 1 only) ------------------------------------------ */
static SlipDecoder slip;

static uint8_t machine_type = 0xFF;         /* 0=KH910/950, 1=KH900/930/940/965 */
static uint8_t ayab_state  = 0;             /* 0=wait,1=init,2=ready,3=knit */
static uint8_t carriage    = AYAB_CARRIAGE_NONE;
static uint8_t position    = 0;
static uint8_t direction   = AYAB_DIR_NONE;

static uint8_t line_counter = 0;            /* next AYAB line number to request */
static bool    last_line_pending = false;   /* cnfLine carried the last-line flag */
static bool    continuous_reporting = false;
static bool    machine_present = false;     /* machine seen by core 0 */
static uint8_t prefill_pending = 0;         /* rows to store before raising fe7e=D2 */

/* ---- CRC-8 (Dallas/Maxim, reflected poly 0x8C) — the AYAB checksum ------ */
static uint8_t ayab_crc8(const uint8_t *buf, size_t len) {
    uint8_t crc = 0;
    while (len--) {
        uint8_t extract = *buf++;
        for (uint8_t i = 8; i; i--) {
            uint8_t sum = (crc ^ extract) & 1;
            crc >>= 1;
            if (sum) crc ^= 0x8C;
            extract >>= 1;
        }
    }
    return crc;
}

/* ---- USB CDC output ------------------------------------------------------ */
static void cdc_write(const uint8_t *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        uint32_t chunk = (uint32_t)(len - sent);
        if (chunk > 64) chunk = 64;
        uint32_t w = tud_cdc_write(data + sent, chunk);
        sent += w;
        if (w == 0) {
            /* TX FIFO full (host stalled): let TinyUSB drain it, then retry. */
            tight_loop_contents();
            tud_task();
        }
    }
    tud_cdc_write_flush();
}

static uint8_t slipbuf[160];

static void ayab_send(const uint8_t *payload, size_t len) {
    size_t n = slip_encode(slipbuf, sizeof(slipbuf), payload, len);
    if (n) cdc_write(slipbuf, n);
}

/* ---- outbound messages --------------------------------------------------- */
static void send_cnfInfo(void) {
    /* cnfInfo: token, API version, fw major/minor/patch, 16-byte suffix + NUL */
    uint8_t p[22];
    p[0] = AYAB_cnfInfo;
    p[1] = AYAB_API_VERSION;
    p[2] = AYAB_FW_MAJ;
    p[3] = AYAB_FW_MIN;
    p[4] = AYAB_FW_PATCH;
    for (int i = 5; i < 22; i++) p[i] = 0;
    ayab_send(p, sizeof(p));
}

static void send_cnfInit(uint8_t err)  { uint8_t p[2] = {AYAB_cnfInit, err};  ayab_send(p, 2); }
static void send_cnfStart(uint8_t err) { uint8_t p[2] = {AYAB_cnfStart, err}; ayab_send(p, 2); }
static void send_cnfTest(uint8_t err)  { uint8_t p[2] = {AYAB_cnfTest, err};  ayab_send(p, 2); }

static void send_reqLine(uint8_t line) {
    uint8_t p[3] = {AYAB_reqLine, line, AYAB_OK};
    ayab_send(p, 3);
}

static void send_indState(uint8_t err) {
    uint8_t p[10];
    p[0] = AYAB_indState;
    p[1] = err;
    p[2] = ayab_state;          /* firmware state */
    p[3] = 0x00; p[4] = 0x00;   /* left hall value  */
    p[5] = 0x00; p[6] = 0x00;   /* right hall value */
    p[7] = carriage;
    p[8] = position;
    p[9] = direction;
    ayab_send(p, sizeof(p));
}

/* Transition INIT -> READY and tell the desktop it may send reqStart.  The
 * desktop's `_API6_request_start` waits for an indState with error 0 before
 * issuing reqStart, so this single message is what releases the flow. */
static void become_ready(void) {
    if (ayab_state != AYAB_STATE_INIT) return;
    ayab_state = AYAB_STATE_READY;
    send_indState(AYAB_OK);
}

/* ---- inbound SLIP packets ------------------------------------------------ */
static void on_packet(const uint8_t *buf, size_t len) {
    if (len == 0) return;

    switch (buf[0]) {
    case AYAB_reqInfo:
        dbg_puts("AYAB reqInfo\r\n");
        send_cnfInfo();
        break;

    case AYAB_reqTest:
        dbg_puts("AYAB reqTest\r\n");
        /* No hardware test menu on the KH-970; accept and move on. */
        send_cnfTest(AYAB_OK);
        break;

    case AYAB_reqInit: {
        if (len < 3) { send_cnfInit(AYAB_ERR_EXPECTED); break; }
        if (buf[2] != ayab_crc8(buf, 2)) { send_cnfInit(AYAB_ERR_CHECKSUM); break; }
        /* AYAB groups the 200-needle machines as KH-910/950 (0) and
         * KH-900/930/940/965 (1); both are 200-needle / 25-byte machines and
         * are electrically identical for the CSI translation, so accept both.
         * KH-270 (2) is a 112-needle machine and is rejected. */
        if (buf[1] > 1) { dbg_printf("reqInit bad machine=%u\r\n", buf[1]); send_cnfInit(AYAB_ERR_MACHINE_TYPE); break; }
        machine_type = buf[1];
        ayab_state = AYAB_STATE_INIT;
        last_line_pending = false;
        dbg_printf("AYAB reqInit machine=%u -> ok\r\n", (unsigned)buf[1]);
        send_cnfInit(AYAB_OK);
        /* If the machine is already present (it booted before we connected),
         * release the desktop immediately; otherwise core 1 waits for the
         * EV_CONFIG event from core 0. */
        if (machine_present) become_ready();
        break;
    }

    case AYAB_reqStart: {
        if (len < 5) { send_cnfStart(AYAB_ERR_EXPECTED); break; }
        if (buf[4] != ayab_crc8(buf, 4)) { send_cnfStart(AYAB_ERR_CHECKSUM); break; }
        if (ayab_state != AYAB_STATE_READY) { send_cnfStart(AYAB_ERR_WRONG_STATE); break; }
        continuous_reporting = (buf[3] & 1) != 0;
        /* (buf[3] & 2) = hardware beeper; the KH-970 has no solenoid beeper,
         * so it is accepted and ignored. */

        dbg_printf("AYAB reqStart (start=%u stop=%u flags=%u)\r\n",
                   (unsigned)buf[1], (unsigned)buf[2], (unsigned)buf[3]);
        ayab_state = AYAB_STATE_KNIT;
        line_counter = 0;
        last_line_pending = false;

        cmd_reset();
        /* Pre-fill the first two rows (A0/A1) before raising fe7e = 0xD2.
         * fe7e=D2 is what makes the machine leave its boot wait and actually
         * start knitting; without it the machine never sends 0xB0/0xB1/0xB2
         * and no needles are selected.  cmd_ready() is raised once both rows
         * have arrived (see cnfLine).
         *
         * ORDER MATTERS: send cnfStart BEFORE the reqLine requests.  The
         * desktop is in CONFIRM_START until it receives cnfStart and drops
         * every other message, so reqLine sent first would be lost and the
         * desktop would wait forever for a line request. */
        prefill_pending = 2;
        send_cnfStart(AYAB_OK);
        send_reqLine(line_counter++);   /* line 0 */
        send_reqLine(line_counter++);   /* line 1 */
        break;
    }

    case AYAB_cnfLine: {
        if (len < 4 + AYAB_LINE_LEN + 1) break;
        uint8_t line  = buf[1];
        /* buf[2] = color (unused: the machine knits one colour per pass) */
        uint8_t flags = buf[3];
        uint8_t crc   = buf[4 + AYAB_LINE_LEN];
        if (crc != ayab_crc8(buf, 4 + AYAB_LINE_LEN)) break;  /* drop; host resends on timeout */

        uint8_t row[AYAB_LINE_LEN];
        for (int i = 0; i < AYAB_LINE_LEN; i++) {
#if AYAB_INVERT_LINE_DATA
            row[i] = (uint8_t)~buf[4 + i];
#else
            row[i] = buf[4 + i];
#endif
        }
        cmd_set_row(line, row);
        dbg_printf("cnfLine %u (flags=%u) firstbyte=%02X\r\n",
                   (unsigned)line, (unsigned)flags, (unsigned)row[0]);
        if (flags & 1) last_line_pending = true;

        /* Once the first two rows are cached, raise fe7e = 0xD2 so the machine
         * leaves its boot wait and starts knitting (and sending 0xB0/0xB1/0xB2). */
        if (prefill_pending > 0) {
            prefill_pending--;
            if (prefill_pending == 0) {
                dbg_puts("prefill done -> cmd_ready()\r\n");
                cmd_ready();
            }
        }
        break;
    }

    default:
        break;
    }
}

/* ---- core-0 events ------------------------------------------------------- */
static void handle_event(uint8_t code, uint8_t arg) {
    switch (code) {
    case EV_CONFIG:              /* machine present */
        dbg_printf("EV_CONFIG machine present (arg=%u)\r\n", (unsigned)arg);
        machine_present = true;
        become_ready();
        break;

    case EV_MODE_KNIT:   dbg_puts("EV MODE_KNIT\r\n");   carriage = AYAB_CARRIAGE_KNIT;   break;
    case EV_MODE_LACE:   dbg_puts("EV MODE_LACE\r\n");   carriage = AYAB_CARRIAGE_LACE;   break;
    case EV_MODE_GARTER: dbg_puts("EV MODE_GARTER\r\n"); carriage = AYAB_CARRIAGE_GARTER; break;

    case EV_B1:          /* forward sensor */
        dbg_puts("EV B1\r\n");
        /* Sensor geometry is approximate; position is cosmetic for the
         * desktop's progress bar.  See README for calibration notes. */
        position = 60;
        direction = AYAB_DIR_RIGHT;
        if (ayab_state == AYAB_STATE_KNIT && continuous_reporting) send_indState(AYAB_OK);
        break;

    case EV_B2:          /* back sensor */
        dbg_puts("EV B2\r\n");
        position = 190;
        direction = AYAB_DIR_LEFT;
        if (ayab_state == AYAB_STATE_KNIT && continuous_reporting) send_indState(AYAB_OK);
        break;

    case EV_B0:          /* row complete / advance (arg = new row index) */
        dbg_printf("EV B0 row=%u\r\n", (unsigned)arg);
        position = (direction == AYAB_DIR_LEFT) ? 0 : 255;
        if (ayab_state == AYAB_STATE_KNIT) {
            if (!last_line_pending) {
                dbg_printf("reqLine %u\r\n", (unsigned)line_counter);
                send_reqLine(line_counter++);
            } else {
                /* The final (blank) line was knitted: stop requesting and
                 * return to the init state for the next job. */
                dbg_puts("last line done -> init\r\n");
                last_line_pending = false;
                ayab_state = AYAB_STATE_INIT;
            }
        }
        if (continuous_reporting) send_indState(AYAB_OK);
        break;
    }
}

/* ---- core 1 entry -------------------------------------------------------- */
void ayab_core1_main(void) {
    slip_decoder_init(&slip);
    dbg_puts("rp2040ayab core1 up\r\n");

    while (1) {
        tud_task();

        /* USB -> SLIP -> AYAB packets */
        uint8_t buf[64];
        while (tud_cdc_available()) {
            uint32_t n = tud_cdc_read(buf, sizeof(buf));
            for (uint32_t i = 0; i < n; i++) {
                const uint8_t *pkt;
                size_t len;
                if (slip_feed(&slip, buf[i], &pkt, &len)) on_packet(pkt, len);
            }
        }

        /* core 0 -> translation */
        uint8_t code, arg;
        while (ev_pop(&code, &arg)) handle_event(code, arg);

        /* Flush the core-0 debug ring to the debug CDC port. */
        dbg_flush();

        tight_loop_contents();
    }
}
