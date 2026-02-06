#ifndef _UTIL_PARITY_H_
#define _UTIL_PARITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#define parity_even_bit(val) ({ \
    unsigned int __val = (val); \
    __val ^= __val >> 4; \
    __val &= 0xf; \
    (0x6996 >> __val) & 1; \
})

#ifdef __cplusplus
}
#endif

#endif
