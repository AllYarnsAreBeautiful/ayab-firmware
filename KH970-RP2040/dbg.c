/* Debug logging implementation (compiled to nothing without
 * RP2040AYAB_DEBUG — see dbg.h). */
#include "dbg.h"

#ifdef RP2040AYAB_DEBUG

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "pico/multicore.h"
#include "hardware/sync.h"
#include "tusb.h"

#define DBG_CDC_ITF    1          /* CDC interface index for debug output */
#define DBG_RING_SIZE  2048       /* core0 -> core1 byte ring */

static uint8_t dbg_ring[DBG_RING_SIZE];
static volatile uint32_t dbg_head;   /* producer (core 0) */
static volatile uint32_t dbg_tail;   /* consumer (core 1) */

/* Write bytes straight to CDC 1 (core 1 only; TinyUSB is not thread-safe). */
static void dbg_cdc_write(const uint8_t *p, size_t n) {
    if (!tud_cdc_n_connected(DBG_CDC_ITF)) return;
    size_t sent = 0;
    while (sent < n) {
        uint32_t w = tud_cdc_n_write(DBG_CDC_ITF, p + sent, (uint32_t)(n - sent));
        sent += w;
        if (w == 0) { tight_loop_contents(); tud_task(); }
    }
    tud_cdc_n_write_flush(DBG_CDC_ITF);
}

/* Append bytes to the SPSC ring (core 0 producer; drop on overflow). */
static void dbg_enqueue(const uint8_t *p, size_t n) {
    uint32_t head = dbg_head;
    uint32_t tail = dbg_tail;
    size_t room = DBG_RING_SIZE - (head - tail) - 1;
    if (n > room) n = room;
    for (size_t i = 0; i < n; i++) dbg_ring[head++ & (DBG_RING_SIZE - 1)] = p[i];
    __dmb();
    dbg_head = head;
}

static void dbg_emit(const uint8_t *p, size_t n) {
    if (get_core_num() == 1) dbg_cdc_write(p, n);
    else dbg_enqueue(p, n);
}

void dbg_putc(char c) { dbg_emit((const uint8_t *)&c, 1); }

void dbg_puts(const char *s) { dbg_emit((const uint8_t *)s, strlen(s)); }

void dbg_hex8(uint8_t b) {
    static const char hex[] = "0123456789ABCDEF";
    char buf[3] = { hex[b >> 4], hex[b & 0x0F], ' ' };
    dbg_emit((const uint8_t *)buf, 3);
}

void dbg_hex16(uint16_t v) {
    dbg_hex8((uint8_t)(v >> 8));
    dbg_hex8((uint8_t)(v & 0xFF));
}

void dbg_printf(const char *fmt, ...) {
    char buf[96];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    if (n > 0) dbg_emit((const uint8_t *)buf,
                        (size_t)(n < (int)sizeof(buf) ? n : sizeof(buf) - 1));
}

/* Drain the core-0 ring into CDC 1.  Called from core 1's main loop. */
void dbg_flush(void) {
    uint32_t tail = dbg_tail;
    __dmb();
    uint32_t head = dbg_head;
    if (tail == head) return;
    if (!tud_cdc_n_connected(DBG_CDC_ITF)) {
        /* No debug terminal attached: discard so the ring never wedges. */
        dbg_tail = head;
        return;
    }
    while (tail != head && tud_cdc_n_write_available(DBG_CDC_ITF)) {
        tud_cdc_n_write_char(DBG_CDC_ITF, dbg_ring[tail & (DBG_RING_SIZE - 1)]);
        tail++;
    }
    __dmb();
    dbg_tail = tail;
    tud_cdc_n_write_flush(DBG_CDC_ITF);
}

#endif /* RP2040AYAB_DEBUG */
