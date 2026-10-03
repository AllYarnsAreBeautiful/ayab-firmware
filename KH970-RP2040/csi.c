/* Core 0: CSI master bit-bang.
 *
 * This is a byte-exact port of the proven `rp2040/csi_bridge.c` core-0 half
 * (verified against a live 30 s capture of the real KH-970), trimmed of the
 * emulator's USB T_STATE/T_EVENT transport.  It still owns the wire: it
 * answers 0x80/0x81 keep-alives, serves needle rows A0/A1 from the shared
 * pattern buffer, and reports the machine's events (START / row sensors /
 * carriage mode / boot) to core 1 through the shared event ring.
 *
 * Pin mapping (matches the KH-970 level shifter used by the emulator):
 *   SCK=4 (clock out), CS=5 (attention in), DIN=7 (machine data in),
 *   DOUT=8 (reply out).  GPIO 6 is reserved (previously drove the
 *   level-shifter VCCA rail and is no longer used).
 */
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "shared.h"
#include "csi.h"
#include "dbg.h"

#define PIN_SCK   4
#define PIN_CS    5
#define PIN_DIN   7
#define PIN_DOUT  8
/* GPIO 6: reserved — was VCCA (level-shifter low rail), no longer driven. */

/* ---- transaction state machine -------------------------------------------
 * One CSI byte = one CS pulse; a framed transaction is:
 *   byte1  M=command      R=0xE2 (idle marker)
 *   byte2  M=0xE2 (sync)  R=command (echo)
 *   byte3  M=0xE2 (pull)  R=0xE0 (control) | fe7e (ack) | data[0]
 *   ...    M=echo         R=data[k+1] ... | 0xE1 (done)
 */
enum { ST_IDLE, ST_SYNC, ST_CONTROL, ST_ACK, ST_DATA, ST_DONE };

static int   st = ST_IDLE;
static uint8_t cmd;
static const volatile uint8_t *data_ptr;
static uint8_t data_idx;
static uint8_t data_count;

/* fe7e / DOUT re-arm machinery (ports the bridge's proven behaviour). */
static volatile uint8_t  fe7e_ready = 0;
static volatile uint8_t  dout_idle = 1;
static volatile uint8_t  rearm_pending = 0;
static volatile uint32_t rearm_at = 0;
static volatile uint8_t  machine_seen = 0;   /* config relay OR first 0x80 poll */

static uint8_t stage_reply(void) {
    switch (st) {
    case ST_IDLE:    return 0xE2;
    case ST_SYNC:    return cmd;
    case ST_CONTROL: return 0xE0;
    case ST_ACK:     return (cmd == 0x81) ? (uint8_t)(g_knit_flag ? 0xD8 : 0xD9) : g_fe7e;
    case ST_DATA:    return data_ptr[data_idx];
    case ST_DONE:    return 0xE1;
    }
    return 0xE1;
}

static void classify_cmd(void) {
    uint8_t c = cmd;
    if (c == 0x80 || c == 0x81) {
        st = ST_ACK;
    } else if (c >= 0x50 && c <= 0x53) {
        data_ptr = &g_counters[c - 0x50]; data_count = 1; data_idx = 0; st = ST_DATA;
    } else if (c >= 0x90 && c <= 0x92) {
        data_ptr = &g_row_codes[c - 0x90]; data_count = 1; data_idx = 0; st = ST_DATA;
    } else if (c == 0xA0) {
        data_ptr = &g_pattern[g_row_idx % N_ROWS][0]; data_count = N_NEEDLE; data_idx = 0; st = ST_DATA;
    } else if (c == 0xA1) {
        data_ptr = &g_pattern[(g_row_idx + 1) % N_ROWS][0]; data_count = N_NEEDLE; data_idx = 0; st = ST_DATA;
    } else {
        st = ST_CONTROL;
    }
}

/* Report machine presence once (config relay or first keep-alive poll). */
static void mark_machine_present(uint8_t arg) {
    if (!machine_seen) {
        machine_seen = 1;
        ev_push(EV_CONFIG, arg);
    }
}

static void dispatch_cmd(void) {
    /* Log the command byte (keep-alives 0x80/0x81 are far too frequent to
     * log; they are summarised by their counters elsewhere if needed). */
    switch (cmd) {
    case 0xB0:   /* START: carriage reached the far end, row advances */
        dbg_printf("B0 START row=%u\r\n", (unsigned)g_row_idx);
        g_fe7e = 0xD6;
        g_knit_flag = 0;
        g_row_idx = (uint8_t)((g_row_idx + 1) % N_ROWS);
        ev_push(EV_B0, g_row_idx);
        g_fe7e = 0xD0;             /* re-arm for the next knit start */
        rearm_pending = 1;
        break;

    case 0x91:   /* row-code read */
        dbg_puts("91 ROWCODE\r\n");
        g_fe7e = 0xD1;
        break;

    case 0xB1:   /* row-forward sensor */
        dbg_puts("B1 ROW_FWD\r\n");
        g_knit_flag = 1;
        ev_push(EV_B1, 0);
        break;

    case 0xB2:   /* row-back sensor */
        dbg_puts("B2 ROW_BACK\r\n");
        ev_push(EV_B2, 0);
        break;

    case 0xB6: dbg_puts("B6 MODE_KNIT\r\n");   ev_push(EV_MODE_KNIT, 0);   break;
    case 0xB7: dbg_puts("B7 MODE_LACE\r\n");   ev_push(EV_MODE_LACE, 0);   break;
    case 0xB8: dbg_puts("B8 MODE_GARTER\r\n"); ev_push(EV_MODE_GARTER, 0); break;

    case 0x80:   /* keep-alive poll */
        if (rearm_pending == 1) {
            dout_idle = 0;
            rearm_at = time_us_32();
            rearm_pending = 2;
        }
        if (!fe7e_ready) dout_idle = 0;
        mark_machine_present(cmd);
        break;

    case 0x81:   /* knit-flag poll */
        mark_machine_present(cmd);
        break;

    default:
        /* The version-major nibble (0x00-0x0F) marks a fresh machine boot:
         * re-arm to 0xD0 and signal core 1 that the machine is present. */
        if (cmd <= 0x0F) {
            dbg_printf("BOOT config %02X\r\n", (unsigned)cmd);
            g_fe7e = 0xD0;
            fe7e_ready = 0;
            dout_idle = 1;
            mark_machine_present(cmd);
        }
        break;
    }
}

static uint8_t csi_step(uint8_t m) {
    switch (st) {
    case ST_IDLE:
        cmd = m;
        st = ST_SYNC;
        break;
    case ST_SYNC:
        if (m != 0xE2) { st = ST_IDLE; break; }
        classify_cmd();
        break;
    case ST_CONTROL:
    case ST_ACK:
        st = ST_DONE;                     /* ack was pulled by the machine */
        break;
    case ST_DATA:
        data_idx++;
        if (data_idx >= data_count) st = ST_DONE;
        break;
    case ST_DONE:
        dispatch_cmd();
        st = ST_IDLE;
        break;
    }
    return stage_reply();
}

/* Bit-bang one full-duplex CSI byte (SCK period ~113 us).  See csi_bridge.c
 * for the timing derivation: reply presented before each SCK rising edge,
 * master byte sampled on each SCK falling edge (DIN is active-low). */
static uint8_t bitbang_byte(uint8_t reply) {
    uint8_t m = 0;
    for (int k = 0; k < 8; k++) {
        gpio_put(PIN_DOUT, (reply >> (7 - k)) & 1);
        gpio_put(PIN_SCK, 1);
        sleep_us(56);
        gpio_put(PIN_SCK, 0);
        m = (uint8_t)((m << 1) | (gpio_get(PIN_DIN) ? 1 : 0));
        sleep_us(56);
    }
    gpio_put(PIN_DOUT, 1);   /* idle DOUT high (armed) */
    return m;
}

/* Wait for the machine's power-on attention (CS low), debounced.  The first
 * byte we clock IS the link-up: its first SCK rising edge is the ack. */
static void csi_handshake(void) {
    gpio_init(PIN_DOUT);
    gpio_set_dir(PIN_DOUT, GPIO_OUT);
    gpio_put(PIN_DOUT, 0);

    gpio_init(PIN_SCK);
    gpio_set_dir(PIN_SCK, GPIO_OUT);
    gpio_put(PIN_SCK, 0);

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_IN);
    gpio_pull_up(PIN_CS);

    gpio_init(PIN_DIN);
    gpio_set_dir(PIN_DIN, GPIO_IN);
    gpio_pull_up(PIN_DIN);

    for (;;) {
        while (gpio_get(PIN_CS)) { tight_loop_contents(); }
        sleep_us(20);
        if (gpio_get(PIN_CS)) continue;
        sleep_us(20);
        if (gpio_get(PIN_CS)) continue;
        break;
    }
}

void csi_run(void) {
    /* Pre-fill the pattern with 0xAA (alternate needles) so the machine never
     * reads an uninitialized row if core 1 is late; core 1 overwrites rows
     * with real AYAB data before raising fe7e = 0xD2. */
    for (int r = 0; r < N_ROWS; r++)
        for (int i = 0; i < N_NEEDLE; i++)
            g_pattern[r][i] = 0xAA;

    csi_handshake();

    uint8_t  next_reply = 0xE2;
    uint32_t last_byte_at = 0;

    while (1) {
        /* Service core-1 commands (cheap; only change state when flagged). */
        if (cmd_poll_reset()) {
            dbg_puts("RESET row=0 fe7e=D0\r\n");
            g_row_idx = 0;
            g_fe7e = 0xD0;
            fe7e_ready = 0;
            rearm_pending = 0;
            dout_idle = 1;
        }
        if (cmd_poll_ready()) {
            dbg_puts("READY fe7e=D2\r\n");
            g_fe7e = 0xD2;
            fe7e_ready = 1;
            dout_idle = 1;
            rearm_pending = 0;
        }
        cmd_poll_set_row();   /* copies any staged row into g_pattern */

        if (!gpio_get(PIN_CS)) {
            /* A long silence (>100 ms) means the machine re-attentioned (link
             * dropped / powered back on): reset the transaction state machine. */
            if (time_us_32() - last_byte_at > 100000) {
                st = ST_IDLE;
                next_reply = 0xE2;
            }
            sleep_us(110);                     /* CS fall -> first clock */
            uint8_t raw = bitbang_byte(next_reply);
            while (!gpio_get(PIN_CS)) { tight_loop_contents(); }
            uint8_t master = raw ^ 0xFF;       /* DIN is active-low */
            next_reply = csi_step(master);
            last_byte_at = time_us_32();
        } else {
            /* Idle (CS high): hold DOUT at the armed state, applying the
             * post-re-arm 2 ms delay that raises fe7e back to 0xD2. */
            if (st == ST_IDLE) {
                if (rearm_pending == 2 && time_us_32() - rearm_at > 2000) {
                    g_fe7e = 0xD2;
                    dout_idle = 1;
                    rearm_pending = 0;
                }
                gpio_put(PIN_DOUT, dout_idle ? 1 : 0);
            }
        }
        tight_loop_contents();
    }
}
