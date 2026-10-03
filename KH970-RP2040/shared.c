/* Cross-core shared state and lock-free SPSC rings.
 *
 * Each ring has a single producer and a single consumer running on opposite
 * cores, so no locking is needed; memory barriers keep the accesses ordered.
 */
#include "shared.h"
#include "hardware/sync.h"   /* __dmb() */

/* ---- pattern + machine-link state --------------------------------------- */
volatile uint8_t g_pattern[N_ROWS][N_NEEDLE];
volatile uint8_t g_row_idx;
volatile uint8_t g_fe7e = 0xD0;
volatile uint8_t g_knit_flag;
volatile uint8_t g_counters[4] = {10, 12, 0, 0};
volatile uint8_t g_row_codes[3] = {0, 0, 0};

/* ---- core0 -> core1 event ring (2-byte records) ------------------------- */
#define EV_RING_SIZE 512u
static volatile uint8_t  ev_ring[EV_RING_SIZE];
static volatile uint32_t ev_head;   /* producer (core0) */
static volatile uint32_t ev_tail;   /* consumer (core1) */

void ev_push(uint8_t code, uint8_t arg) {
    uint32_t head = ev_head;
    uint32_t tail = ev_tail;
    /* drop when full: an event is a progress hint, losing one is harmless */
    if (head - tail + 2 > EV_RING_SIZE) return;
    ev_ring[head & (EV_RING_SIZE - 1)] = code;
    ev_ring[(head + 1) & (EV_RING_SIZE - 1)] = arg;
    __dmb();
    ev_head = head + 2;
}

bool ev_pop(uint8_t *code, uint8_t *arg) {
    uint32_t tail = ev_tail;
    __dmb();
    uint32_t head = ev_head;
    if (head == tail) return false;
    *code = ev_ring[tail & (EV_RING_SIZE - 1)];
    *arg  = ev_ring[(tail + 1) & (EV_RING_SIZE - 1)];
    __dmb();
    ev_tail = tail + 2;
    return true;
}

/* ---- core1 -> core0 commands ----------------------------------------------
 * A row is staged by core1 (25 bytes), then flagged; core0 copies it into the
 * pattern buffer in its idle loop so a row write never races a wire read. */
static volatile uint8_t  stage_data[N_NEEDLE];
static volatile uint8_t  stage_index;
static volatile uint8_t  stage_pending;
static volatile uint8_t  reset_flag;
static volatile uint8_t  ready_flag;

void cmd_set_row(uint8_t row, const uint8_t *data) {
    for (int i = 0; i < N_NEEDLE; i++) stage_data[i] = data[i];
    stage_index = row;
    __dmb();
    stage_pending = 1;
}

void cmd_reset(void) { reset_flag = 1; }
void cmd_ready(void) { ready_flag = 1; }

bool cmd_poll_set_row(void) {
    if (!stage_pending) return false;
    uint8_t r = stage_index % N_ROWS;
    for (int i = 0; i < N_NEEDLE; i++) g_pattern[r][i] = stage_data[i];
    __dmb();
    stage_pending = 0;
    return true;
}

bool cmd_poll_reset(void) {
    if (!reset_flag) return false;
    reset_flag = 0;
    return true;
}

bool cmd_poll_ready(void) {
    if (!ready_flag) return false;
    ready_flag = 0;
    return true;
}
