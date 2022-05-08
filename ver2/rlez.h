#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

bool rlez_encode(void *dst, uint32_t *dst_size, const void *src, const uint32_t src_size);
bool rlez_decode(void *dst, uint32_t *dst_size, const void *src, const uint32_t src_size);

#ifdef __cplusplus
}
#endif
