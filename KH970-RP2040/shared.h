/* Shared state and cross-core rings between the two firmware halves.
 *
 *   core 0 (csi.c)      — CSI master bit-bang.  Produces events, consumes
 *                         pattern rows and the reset/ready commands.
 *   core 1 (ayab.c)     — TinyUSB + AYAB protocol.  Consumes events, produces
 *                         pattern rows and the reset/ready commands.
 *
 * All cross-core state is `volatile` and protected by memory barriers; the
 * ring FIFOs are single-producer/single-consumer and lock-free.
 */
#ifndef RP2040AYAB_SHARED_H
#define RP2040AYAB_SHARED_H

#include <stdint.h>
#include <stdbool.h>

/* The CSI machine is 200 needles wide: 25 bytes per needle row.  The pattern
 * ring mirrors the CB-1's 22-row layout (fddb = 0x16), which is the wrap
 * bound the machine itself uses for its row pointer. */
#define N_NEEDLE 25
#define N_ROWS   22

/* ---- CSI pattern buffer (core1 writes, core0 reads) --------------------- */
/* A0 = g_pattern[g_row_idx], A1 = g_pattern[(g_row_idx + 1) % N_ROWS].
 * core1 pre-fills rows two ahead of g_row_idx so both are always valid when
 * the machine reads them. */
extern volatile uint8_t g_pattern[N_ROWS][N_NEEDLE];
extern volatile uint8_t g_row_idx;      /* advanced by core0 on 0xB0 (START) */

/* ---- machine-link state owned by core0 --------------------------------- */
extern volatile uint8_t g_fe7e;         /* expected ack (0xD0 boot, 0xD2 ready) */
extern volatile uint8_t g_knit_flag;    /* fe67.0 -> 0x81 reply */
extern volatile uint8_t g_counters[4];  /* 0x50-0x53 */
extern volatile uint8_t g_row_codes[3]; /* 0x90-0x92 (knit = 0) */

/* ---- core0 -> core1 events --------------------------------------------- */
#define EV_B0          0x01  /* START: row boundary (arg = new g_row_idx) */
#define EV_B1          0x02  /* ROW_FWD sensor */
#define EV_B2          0x03  /* ROW_BACK sensor */
#define EV_MODE_KNIT   0x04  /* 0xB6 carriage mode */
#define EV_MODE_LACE   0x05  /* 0xB7 carriage mode */
#define EV_MODE_GARTER 0x06  /* 0xB8 carriage mode */
#define EV_CONFIG      0x07  /* machine present (config relay or first poll) */

void ev_push(uint8_t code, uint8_t arg);       /* core0 producer */
bool ev_pop(uint8_t *code, uint8_t *arg);      /* core1 consumer */

/* ---- core1 -> core0 commands ------------------------------------------- */
void cmd_set_row(uint8_t row, const uint8_t *data);  /* fill g_pattern[row] */
void cmd_reset(void);                                  /* row_idx=0, fe7e=D0 */
void cmd_ready(void);                                  /* fe7e=D2 (ready) */

bool cmd_poll_set_row(void);    /* core0: copy any pending row into pattern */
bool cmd_poll_reset(void);      /* core0: consume pending reset */
bool cmd_poll_ready(void);      /* core0: consume pending ready */

#endif
