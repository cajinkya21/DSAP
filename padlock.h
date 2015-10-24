#ifndef MBEDTLS_PADLOCK_H
#define MBEDTLS_PADLOCK_H

#include "aes.h"

#define MBEDTLS_ERR_PADLOCK_DATA_MISALIGNED               -0x0030  /**< Input data should be aligned. */

#if defined(__has_feature)
#if __has_feature(address_sanitizer)
#define MBEDTLS_HAVE_ASAN
#endif
#endif

/* Some versions of ASan result in errors about not enough registers */
#if defined(MBEDTLS_HAVE_ASM) && defined(__GNUC__) && defined(__i386__) && \
    !defined(MBEDTLS_HAVE_ASAN)

#ifndef MBEDTLS_HAVE_X86
#define MBEDTLS_HAVE_X86
#endif

#include <stdint.h>

#define MBEDTLS_PADLOCK_RNG 0x000C
#define MBEDTLS_PADLOCK_ACE 0x00C0
#define MBEDTLS_PADLOCK_PHE 0x0C00
#define MBEDTLS_PADLOCK_PMM 0x3000

#define MBEDTLS_PADLOCK_ALIGN16(x) (uint32_t *) (16 + ((int32_t) x & ~15))

#ifdef __cplusplus
extern "C" {
#endif


int mbedtls_padlock_has_support( int feature );


int mbedtls_padlock_xcryptecb( mbedtls_aes_context *ctx,
                       int mode,
                       const unsigned char input[16],
                       unsigned char output[16] );

int mbedtls_padlock_xcryptcbc( mbedtls_aes_context *ctx,
                       int mode,
                       size_t length,
                       unsigned char iv[16],
                       const unsigned char *input,
                       unsigned char *output );

#ifdef __cplusplus
}
#endif

#endif /* HAVE_X86  */

#endif /* padlock.h */
