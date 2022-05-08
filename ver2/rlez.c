#include <stdio.h>
#include <stdbool.h>

#include "rlez.h"

static const uint8_t ESC_CHAR = 0xaa;

// returns true if out of room
static inline bool rlez_safe_add(uint8_t **pptr, uint8_t const * const max_ptr, uint8_t a) {
    if (*pptr < max_ptr) {
        *(*pptr)++ = a;
        return false;
    }
    return true;
}

static inline void rlez_unsafe_add(uint8_t **pptr, uint8_t a) {
    *(*pptr)++ = a;
}

bool rlez_encode(void *dst, uint32_t *dst_size, const void *src, const uint32_t src_size) {

    const uint8_t *psrc = (const uint8_t *)src;
    uint8_t       *pdst = (uint8_t *)dst;

    uint8_t * const pdstend = pdst + *dst_size; 
    const uint8_t * const psrcend = psrc + src_size; 

    uint32_t zcount = 0;

    while ( (psrc < psrcend) && (pdst < pdstend)) {
        bool add_fail = false;
        uint8_t b = *psrc++;
        if (zcount) {
            if (b == 0) {
                zcount++;
                if (zcount == 255) {
                    rlez_unsafe_add(&pdst, zcount);
                    zcount = 0;
                }
            } else {
                rlez_unsafe_add(&pdst, zcount);
                zcount = 0; 
                if (b == ESC_CHAR) {
                    add_fail |= rlez_safe_add(&pdst, pdstend, ESC_CHAR);
                    add_fail |= rlez_safe_add(&pdst, pdstend, 0);
                } else {
                    add_fail |= rlez_safe_add(&pdst, pdstend, b);
                }
            }
        } else {
            if (b == ESC_CHAR) {
                rlez_unsafe_add(&pdst, ESC_CHAR);
                add_fail |= rlez_safe_add(&pdst, pdstend, 0);
            } else if (b == 0) {
                zcount = 1;
                rlez_unsafe_add(&pdst, ESC_CHAR);
            } else {
                rlez_unsafe_add(&pdst, b);
            }
        }
        if (add_fail) {
            break;
        }
    }

    if (zcount && (pdst <= pdstend)) {
        rlez_unsafe_add(&pdst, zcount);
    }

    bool ok = (psrc == psrcend) && (pdst <= pdstend);

    if (ok) {
        *dst_size = pdst - (uint8_t *)dst;
    }
    return ok;
}


bool rlez_decode(void *dst, uint32_t *dst_size, const void *src, const uint32_t src_size) {
    const uint8_t *psrc = (const uint8_t *)src;
    uint8_t *pdst = (uint8_t *)dst;
   
    uint8_t * const pdstend = pdst + *dst_size; 
    const uint8_t * const psrcend = psrc + src_size; 

    while ((psrc < psrcend) && (pdst < pdstend)) {

        uint8_t b = *psrc++;
        if (b == ESC_CHAR) {
            b = *psrc++;
            if (b == 0)  {
                *(pdst++) = ESC_CHAR;
            } else {
                while (b) {
                    rlez_safe_add(&pdst, pdstend, 0);
                    b--;
                }
            }
        } else {
            *(pdst++) = b;
        }
    }

    bool ok = (psrc == psrcend) && (pdst <= pdstend);

    if (ok) {
        *dst_size = pdst - (uint8_t *)dst;
    }
    return ok;
}


