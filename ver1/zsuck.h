#pragma once
#include <stdint.h>
#include <stdbool.h>

#define ZSUCK_ZC_TYPE uint16_t

typedef struct zsuck_decode_context_t {
   bool    active;
   uint8_t bytecount;
   uint8_t bytes[sizeof(ZSUCK_ZC_TYPE)];
} zsuck_decode_context_t;

uint32_t zsuck_encode(void *dst, uint32_t dst_max, uint32_t *dst_count,
                      const void *src, uint32_t srclen);

uint32_t zsuck_decode(void *dst, uint32_t dst_max, uint32_t *dst_count,
                      const void *src, uint32_t srclen);

uint32_t zsuck_decode_chunk(void *dst, uint32_t dst_max, uint32_t *dst_count,
                            const void *src, uint32_t srclen,
                            zsuck_decode_context_t *ctx);

void zsuck_show_ctx(const zsuck_decode_context_t *c, const char *msg);
