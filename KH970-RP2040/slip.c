#include "slip.h"

size_t slip_encode(uint8_t *dst, size_t cap, const uint8_t *payload, size_t len) {
    size_t n = 0;

    /* worst case: every byte escaped (2x) plus two END bytes */
    if (cap < 2 + 2 * len + 2) return 0;

    dst[n++] = SLIP_END;
    for (size_t i = 0; i < len; i++) {
        uint8_t b = payload[i];
        if (b == SLIP_END) {
            dst[n++] = SLIP_ESC;
            dst[n++] = SLIP_ESC_END;
        } else if (b == SLIP_ESC) {
            dst[n++] = SLIP_ESC;
            dst[n++] = SLIP_ESC_ESC;
        } else {
            dst[n++] = b;
        }
    }
    dst[n++] = SLIP_END;
    return n;
}

void slip_decoder_init(SlipDecoder *d) {
    d->len = 0;
    d->escaping = false;
}

bool slip_feed(SlipDecoder *d, uint8_t b, const uint8_t **pkt, size_t *len) {
    if (b == SLIP_END) {
        /* Frame delimiter: deliver whatever was buffered (if any).  A leading
         * or stray END simply delivers nothing and resets the buffer. */
        if (d->len > 0) {
            *pkt = d->buf;
            *len = d->len;
            d->len = 0;
            d->escaping = false;
            return true;
        }
        d->escaping = false;
        return false;
    }

    if (d->escaping) {
        if (b == SLIP_ESC_END) b = SLIP_END;
        else if (b == SLIP_ESC_ESC) b = SLIP_ESC;
        /* any other byte after ESC is a protocol error; keep it verbatim */
        d->escaping = false;
    } else if (b == SLIP_ESC) {
        d->escaping = true;
        return false;
    }

    if (d->len < sizeof(d->buf)) d->buf[d->len++] = b;
    return false;
}
