/*****************************************************************************
 * Copyright (C) Ajinkya G.C cajinkya21@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef MBEDTLS_AESNI_H
#define MBEDTLS_AESNI_H

#include "aes.h"

#define MBEDTLS_AESNI_AES      0x02000000u
#define MBEDTLS_AESNI_CLMUL    0x00000002u

#if defined(MBEDTLS_HAVE_ASM) && defined(__GNUC__) &&  \
    ( defined(__amd64__) || defined(__x86_64__) )   &&  \
    ! defined(MBEDTLS_HAVE_X86_64)
#define MBEDTLS_HAVE_X86_64
#endif

#if defined(MBEDTLS_HAVE_X86_64)

#ifdef __cplusplus
extern "C" {
#endif

int mbedtls_aesni_has_support( unsigned int what );

int mbedtls_aesni_crypt_ecb( mbedtls_aes_context *ctx,
                     int mode,
                     const unsigned char input[16],
                     unsigned char output[16] );

void mbedtls_aesni_gcm_mult( unsigned char c[16],
                     const unsigned char a[16],
                     const unsigned char b[16] );

void mbedtls_aesni_inverse_key( unsigned char *invkey,
                        const unsigned char *fwdkey, int nr );

int mbedtls_aesni_setkey_enc( unsigned char *rk,
                      const unsigned char *key,
                      size_t bits );

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_HAVE_X86_64 */

#endif /* MBEDTLS_AESNI_H */
