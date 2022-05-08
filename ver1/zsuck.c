
// Author   : Dave J
// Date     : Oct 2020
// Varsion  : 0.0.1
// License  : you as you like
// Warranty : none whatsoever

#include <string.h>
#include <stdio.h>

#include "zsuck.h"

static uint32_t zsuck_safe_add(uint8_t **dst, const void *dst_max, const void *src, uint32_t src_len) {
    uint32_t remaining = 0;
    uint32_t avail = (uint8_t *)dst_max - *dst;
    if (src_len > avail) {
        remaining = src_len - avail;
        src_len = avail;
    }
    memcpy(*dst, src, src_len);
    (*dst) += src_len;
    return remaining;
}

static uint32_t zsuck_safe_add_byte(uint8_t **dst, const uint8_t *dst_max, uint8_t b) {
    if (dst_max > *dst) {
        **dst = b;
        (*dst)++;
        return 0;
    }
    return 1;
}

static uint32_t zsuck_safe_add_zeros(uint8_t **dst, const void *dst_max, uint32_t count) {
    uint32_t remaining = 0;
    uint32_t avail = (uint8_t *)dst_max - *dst;
    if (count > avail) {
        remaining = count - avail;
        count = avail;
    }
    memset(*dst, 0,  count);
    *dst += count;
    return remaining;
}

void zsuck_show_ctx(const zsuck_decode_context_t *ctx, const char *msg) {
    if (ctx->active) {
        printf("%s ctx->bytecount %u ctx->bytes %02x %02x %02x %02x\n",
            msg,
            ctx->bytecount,
            ctx->bytes[0], ctx->bytes[1], ctx->bytes[2], ctx->bytes[3]);
    } else {
        printf("%s inactive\n", msg);
    }
    fflush(stdout);
}

uint32_t zsuck_encode(void *dst, uint32_t dst_max, uint32_t *dst_count,
                      const void *src, uint32_t srclen) {

    const uint8_t *srcp = (uint8_t *)src;
    const uint8_t *src_end = (uint8_t *)src + srclen;
    const uint8_t *dstp_max = dst + dst_max;
    uint32_t overflow = 0;
    uint8_t *dstp = dst;

    ZSUCK_ZC_TYPE zcount = 0;

    while (srcp < src_end) {
        if (*srcp) {
            if (zcount) {
                overflow += zsuck_safe_add_byte(&dstp,dstp_max, 0);
                overflow += zsuck_safe_add(&dstp, dstp_max, &zcount, sizeof(ZSUCK_ZC_TYPE));
                zcount = 0;
            }
            *dstp = *srcp;
            dstp++;
        } else {
            zcount++;
            // handle overflow 
            if (zcount == (ZSUCK_ZC_TYPE)(-1)) {
                overflow += zsuck_safe_add_byte(&dstp,dstp_max, 0);
                overflow += zsuck_safe_add(&dstp, dstp_max, &zcount, sizeof(ZSUCK_ZC_TYPE));
                zcount = 0;
            }
        }
        srcp++;
    }
    if (zcount) {
        overflow += zsuck_safe_add_byte(&dstp,dstp_max, 0);
        overflow += zsuck_safe_add(&dstp, dstp_max, &zcount, sizeof(ZSUCK_ZC_TYPE));
    }
    *dst_count = dstp - (uint8_t *)dst;
    return overflow;
}


uint32_t zsuck_decode_chunk(void *dst, uint32_t dst_max, uint32_t *dst_count,
                            const void *src, uint32_t srclen, 
                            zsuck_decode_context_t *ctx) {

    const uint8_t *srcp = (uint8_t *)src;
    const uint8_t *src_end = (uint8_t *)src + srclen;
    const uint8_t *dstp_max = dst + dst_max;
    uint8_t *dstp = dst;
    uint32_t overflow = 0;

    while (srcp < src_end) {
        if (ctx->active) {
            if (ctx->bytecount < sizeof(ZSUCK_ZC_TYPE)) {
                ctx->bytes[ctx->bytecount] = *srcp;
                ctx->bytecount++;
            }
            if (ctx->bytecount == sizeof(ZSUCK_ZC_TYPE)) {
                uint32_t zcount = 0;
                memcpy(&zcount, ctx->bytes, sizeof(ZSUCK_ZC_TYPE));
                overflow += zsuck_safe_add_zeros(&dstp, dstp_max, zcount);
                ctx->bytecount = 0;
                ctx->active = false;
            }
        } else if (*srcp) {
            overflow += zsuck_safe_add_byte(&dstp, dstp_max, *srcp);
        } else {
            ctx->active = true;
            ctx->bytecount = 0;
        }
        srcp++; // consume the empty byte
    }
    *dst_count = dstp - (uint8_t *)dst;
    return overflow;
}


uint32_t zsuck_decode(void *dst, uint32_t dst_max, uint32_t *dst_count,
                      const void *src, uint32_t srclen) {
    zsuck_decode_context_t ctx = {};
    uint32_t overflow = zsuck_decode_chunk(dst, dst_max, dst_count, src, srclen, &ctx);
    return overflow;
}

