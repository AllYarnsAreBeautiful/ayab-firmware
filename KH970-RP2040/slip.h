/* Minimal SLIP framing (RFC 1055) for the AYAB API v6 transport.
 *
 * AYAB Desktop uses Python `sliplib`; the reference firmware uses PacketSerial
 * with the SLIP codec.  Both use the standard byte escaping:
 *   END = 0xC0, ESC = 0xDB, ESC_END = 0xDC, ESC_ESC = 0xDD
 * A frame is `END | escaped-payload | END`.
 */
#ifndef RP2040AYAB_SLIP_H
#define RP2040AYAB_SLIP_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SLIP_END     0xC0
#define SLIP_ESC     0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD

/* Encode `payload` into `dst` as one complete SLIP frame (including the
 * leading and trailing END bytes).  Returns the number of bytes written, or 0
 * if the frame would not fit in `cap`. */
size_t slip_encode(uint8_t *dst, size_t cap, const uint8_t *payload, size_t len);

/* Incremental decoder.  Feed received bytes one at a time; when a complete
 * packet is decoded, `*pkt`/`*len` point at it (valid until the next call)
 * and the function returns true.
 *
 * The decoder tolerates BOTH framings used by Python sliplib:
 *   sliplib <  0.7: `END | payload | END`   (leading END present)
 *   sliplib >= 0.7: `payload | END`         (no leading END, default config)
 * Bytes are accumulated between END delimiters regardless of whether a
 * leading END was seen, so neither framing drops the first payload byte. */
typedef struct {
    uint8_t buf[128];
    size_t  len;
    bool    escaping;
} SlipDecoder;

void slip_decoder_init(SlipDecoder *d);
bool slip_feed(SlipDecoder *d, uint8_t byte, const uint8_t **pkt, size_t *len);

#endif
