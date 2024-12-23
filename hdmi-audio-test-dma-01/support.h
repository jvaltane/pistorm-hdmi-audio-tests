#ifndef _SUPPORT_H
#define _SUPPORT_H

#include <stdint.h>

/* from pistorm support.h */
static inline uint64_t LE64(uint64_t x) { return __builtin_bswap64(x); }
static inline uint32_t LE32(uint32_t x) { return __builtin_bswap32(x); }
static inline uint16_t LE16(uint16_t x) { return __builtin_bswap16(x); }

#endif /* _SUPPORT_H */
