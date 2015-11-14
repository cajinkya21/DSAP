/*****************************************************************************
 * Copyright (C) Ajinkya G.C cajinkya21@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3.0 of the License, or
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

#ifndef MBEDTLS_MD_H
#define MBEDTLS_MD_H

#include <stddef.h>

#define MBEDTLS_ERR_MD_FEATURE_UNAVAILABLE                -0x5080  /**< The selected feature is not available. */
#define MBEDTLS_ERR_MD_BAD_INPUT_DATA                     -0x5100  /**< Bad input parameters to function. */
#define MBEDTLS_ERR_MD_ALLOC_FAILED                       -0x5180  /**< Failed to allocate memory. */
#define MBEDTLS_ERR_MD_FILE_IO_ERROR                      -0x5200  /**< Opening or reading of file failed. */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    MBEDTLS_MD_NONE=0,
    MBEDTLS_MD_MD2,
    MBEDTLS_MD_MD4,
    MBEDTLS_MD_MD5,
    MBEDTLS_MD_SHA1,
    MBEDTLS_MD_SHA224,
    MBEDTLS_MD_SHA256,
    MBEDTLS_MD_SHA384,
    MBEDTLS_MD_SHA512,
    MBEDTLS_MD_RIPEMD160,
} mbedtls_md_type_t;

#if defined(MBEDTLS_SHA512_C)
#define MBEDTLS_MD_MAX_SIZE         64  /* longest known is SHA512 */
#else
#define MBEDTLS_MD_MAX_SIZE         32  /* longest known is SHA256 or less */
#endif

/**
 * Opaque struct defined in md_internal.h
 */
typedef struct mbedtls_md_info_t mbedtls_md_info_t;

/**
 * Generic message digest context.
 */
typedef struct {
    /** Information about the associated message digest */
    const mbedtls_md_info_t *md_info;

    /** Digest-specific context */
    void *md_ctx;

    /** HMAC part of the context */
    void *hmac_ctx;
} mbedtls_md_context_t;


const int *mbedtls_md_list( void );


void mbedtls_md_init( mbedtls_md_context_t *ctx );

void mbedtls_md_free( mbedtls_md_context_t *ctx );

#if ! defined(MBEDTLS_DEPRECATED_REMOVED)
#if defined(MBEDTLS_DEPRECATED_WARNING)
#define MBEDTLS_DEPRECATED    __attribute__((deprecated))
#else
#define MBEDTLS_DEPRECATED
#endif

int mbedtls_md_init_ctx( mbedtls_md_context_t *ctx, const mbedtls_md_info_t *md_info ) MBEDTLS_DEPRECATED;
#undef MBEDTLS_DEPRECATED
#endif /* MBEDTLS_DEPRECATED_REMOVED */


int mbedtls_md_setup( mbedtls_md_context_t *ctx, const mbedtls_md_info_t *md_info, int hmac );

int mbedtls_md_clone( mbedtls_md_context_t *dst,
                      const mbedtls_md_context_t *src );

unsigned char mbedtls_md_get_size( const mbedtls_md_info_t *md_info );

mbedtls_md_type_t mbedtls_md_get_type( const mbedtls_md_info_t *md_info );

const char *mbedtls_md_get_name( const mbedtls_md_info_t *md_info );

int mbedtls_md_starts( mbedtls_md_context_t *ctx );

int mbedtls_md_update( mbedtls_md_context_t *ctx, const unsigned char *input, size_t ilen );

int mbedtls_md_finish( mbedtls_md_context_t *ctx, unsigned char *output );

int mbedtls_md( const mbedtls_md_info_t *md_info, const unsigned char *input, size_t ilen,
        unsigned char *output );

#if defined(MBEDTLS_FS_IO)

int mbedtls_md_file( const mbedtls_md_info_t *md_info, const char *path,
                     unsigned char *output );
#endif /* MBEDTLS_FS_IO */

int mbedtls_md_hmac_starts( mbedtls_md_context_t *ctx, const unsigned char *key,
                    size_t keylen );

int mbedtls_md_hmac_update( mbedtls_md_context_t *ctx, const unsigned char *input,
                    size_t ilen );

int mbedtls_md_hmac_finish( mbedtls_md_context_t *ctx, unsigned char *output);

int mbedtls_md_hmac_reset( mbedtls_md_context_t *ctx );

int mbedtls_md_hmac( const mbedtls_md_info_t *md_info, const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char *output );

/* Internal use */
int mbedtls_md_process( mbedtls_md_context_t *ctx, const unsigned char *data );

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_MD_H */
