/* Debug logging on the SECOND USB CDC port (interface 1).
 *
 * AYAB data uses CDC interface 0; debug output uses CDC interface 1 so the
 * two streams never mix.  All logging is compiled out unless
 * `RP2040AYAB_DEBUG` is defined (see CMakeLists.txt:
 * `cmake -B build -DRP2040AYAB_DEBUG=ON`), so a release build has zero
 * overhead and no debug port.
 *
 * The functions may be called from either core:
 *   core 0 (CSI) enqueues bytes into a lock-free SPSC ring;
 *   core 1 (TinyUSB owner) writes directly to CDC 1 and drains the ring.
 * `dbg_flush()` must be called periodically from core 1's main loop.
 */
#ifndef RP2040AYAB_DBG_H
#define RP2040AYAB_DBG_H

#include <stdint.h>
#include <stddef.h>

#ifdef RP2040AYAB_DEBUG
  void dbg_putc(char c);
  void dbg_puts(const char *s);
  void dbg_hex8(uint8_t b);
  void dbg_hex16(uint16_t v);
  void dbg_printf(const char *fmt, ...);
  void dbg_flush(void);   /* core 1 only: drain the core-0 ring to CDC 1 */
#else
  #define dbg_putc(c)     ((void)0)
  #define dbg_puts(s)     ((void)0)
  #define dbg_hex8(b)     ((void)0)
  #define dbg_hex16(v)    ((void)0)
  #define dbg_printf(...) ((void)0)
  #define dbg_flush()     ((void)0)
#endif

#endif
