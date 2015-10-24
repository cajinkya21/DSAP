#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_MD_C)

#include "md.h"
#include "md_internal.h"

#if defined(MBEDTLS_PLATFORM_C)
#include "platform.h"
#else
#include <stdlib.h>
#define mbedtls_calloc    calloc
#define mbedtls_free       free
#endif

#include <string.h>

#if defined(MBEDTLS_FS_IO)
#include <stdio.h>
#endif

/* Implementation that should never be optimized out by the compiler */
static void mbedtls_zeroize( void *v, size_t n ) {
    volatile unsigned char *p = v; while( n-- ) *p++ = 0;
}

/*
 * Reminder: update profiles in x509_crt.c when adding a new hash!
 */
static const int supported_digests[] = {

#if defined(MBEDTLS_SHA512_C)
        MBEDTLS_MD_SHA512,
        MBEDTLS_MD_SHA384,
#endif

#if defined(MBEDTLS_SHA256_C)
        MBEDTLS_MD_SHA256,
        MBEDTLS_MD_SHA224,
#endif

#if defined(MBEDTLS_SHA1_C)
        MBEDTLS_MD_SHA1,
#endif

#if defined(MBEDTLS_RIPEMD160_C)
        MBEDTLS_MD_RIPEMD160,
#endif

#if defined(MBEDTLS_MD5_C)
        MBEDTLS_MD_MD5,
#endif

#if defined(MBEDTLS_MD4_C)
        MBEDTLS_MD_MD4,
#endif

#if defined(MBEDTLS_MD2_C)
        MBEDTLS_MD_MD2,
#endif

        MBEDTLS_MD_NONE
};

const int *mbedtls_md_list( void )
{
    return( supported_digests );
}
void mbedtls_md_init( mbedtls_md_context_t *ctx )
{
    memset( ctx, 0, sizeof( mbedtls_md_context_t ) );
}

void mbedtls_md_free( mbedtls_md_context_t *ctx )
{
    if( ctx == NULL || ctx->md_info == NULL )
        return;

    if( ctx->md_ctx != NULL )
        ctx->md_info->ctx_free_func( ctx->md_ctx );

    if( ctx->hmac_ctx != NULL )
    {
        mbedtls_zeroize( ctx->hmac_ctx, 2 * ctx->md_info->block_size );
        mbedtls_free( ctx->hmac_ctx );
    }

    mbedtls_zeroize( ctx, sizeof( mbedtls_md_context_t ) );
}

int mbedtls_md_clone( mbedtls_md_context_t *dst,
                      const mbedtls_md_context_t *src )
{
    if( dst == NULL || dst->md_info == NULL ||
        src == NULL || src->md_info == NULL ||
        dst->md_info != src->md_info )
    {
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );
    }

    dst->md_info->clone_func( dst->md_ctx, src->md_ctx );

    return( 0 );
}

#if ! defined(MBEDTLS_DEPRECATED_REMOVED)
int mbedtls_md_init_ctx( mbedtls_md_context_t *ctx, const mbedtls_md_info_t *md_info )
{
    return mbedtls_md_setup( ctx, md_info, 1 );
}
#endif

int mbedtls_md_setup( mbedtls_md_context_t *ctx, const mbedtls_md_info_t *md_info, int hmac )
{
    if( md_info == NULL || ctx == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    if( ( ctx->md_ctx = md_info->ctx_alloc_func() ) == NULL )
        return( MBEDTLS_ERR_MD_ALLOC_FAILED );

    if( hmac != 0 )
    {
        ctx->hmac_ctx = mbedtls_calloc( 2, md_info->block_size );
        if( ctx->hmac_ctx == NULL )
        {
            md_info->ctx_free_func( ctx->md_ctx );
            return( MBEDTLS_ERR_MD_ALLOC_FAILED );
        }
    }

    ctx->md_info = md_info;

    return( 0 );
}

int mbedtls_md_starts( mbedtls_md_context_t *ctx )
{
    if( ctx == NULL || ctx->md_info == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    ctx->md_info->starts_func( ctx->md_ctx );

    return( 0 );
}

int mbedtls_md_update( mbedtls_md_context_t *ctx, const unsigned char *input, size_t ilen )
{
    if( ctx == NULL || ctx->md_info == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    ctx->md_info->update_func( ctx->md_ctx, input, ilen );

    return( 0 );
}

int mbedtls_md_finish( mbedtls_md_context_t *ctx, unsigned char *output )
{
    if( ctx == NULL || ctx->md_info == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    ctx->md_info->finish_func( ctx->md_ctx, output );

    return( 0 );
}

int mbedtls_md( const mbedtls_md_info_t *md_info, const unsigned char *input, size_t ilen,
            unsigned char *output )
{
    if( md_info == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    md_info->digest_func( input, ilen, output );

    return( 0 );
}

#if defined(MBEDTLS_FS_IO)
int mbedtls_md_file( const mbedtls_md_info_t *md_info, const char *path, unsigned char *output )
{
    int ret;
    FILE *f;
    size_t n;
    mbedtls_md_context_t ctx;
    unsigned char buf[1024];

    if( md_info == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    if( ( f = fopen( path, "rb" ) ) == NULL )
        return( MBEDTLS_ERR_MD_FILE_IO_ERROR );

    mbedtls_md_init( &ctx );

    if( ( ret = mbedtls_md_setup( &ctx, md_info, 0 ) ) != 0 )
        goto cleanup;

    md_info->starts_func( ctx.md_ctx );

    while( ( n = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
        md_info->update_func( ctx.md_ctx, buf, n );

    if( ferror( f ) != 0 )
    {
        ret = MBEDTLS_ERR_MD_FILE_IO_ERROR;
        goto cleanup;
    }

    md_info->finish_func( ctx.md_ctx, output );

cleanup:
    fclose( f );
    mbedtls_md_free( &ctx );

    return( ret );
}
#endif /* MBEDTLS_FS_IO */

int mbedtls_md_hmac_starts( mbedtls_md_context_t *ctx, const unsigned char *key, size_t keylen )
{
    unsigned char sum[MBEDTLS_MD_MAX_SIZE];
    unsigned char *ipad, *opad;
    size_t i;

    if( ctx == NULL || ctx->md_info == NULL || ctx->hmac_ctx == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    if( keylen > (size_t) ctx->md_info->block_size )
    {
        ctx->md_info->starts_func( ctx->md_ctx );
        ctx->md_info->update_func( ctx->md_ctx, key, keylen );
        ctx->md_info->finish_func( ctx->md_ctx, sum );

        keylen = ctx->md_info->size;
        key = sum;
    }

    ipad = (unsigned char *) ctx->hmac_ctx;
    opad = (unsigned char *) ctx->hmac_ctx + ctx->md_info->block_size;

    memset( ipad, 0x36, ctx->md_info->block_size );
    memset( opad, 0x5C, ctx->md_info->block_size );

    for( i = 0; i < keylen; i++ )
    {
        ipad[i] = (unsigned char)( ipad[i] ^ key[i] );
        opad[i] = (unsigned char)( opad[i] ^ key[i] );
    }

    mbedtls_zeroize( sum, sizeof( sum ) );

    ctx->md_info->starts_func( ctx->md_ctx );
    ctx->md_info->update_func( ctx->md_ctx, ipad, ctx->md_info->block_size );

    return( 0 );
}

int mbedtls_md_hmac_update( mbedtls_md_context_t *ctx, const unsigned char *input, size_t ilen )
{
    if( ctx == NULL || ctx->md_info == NULL || ctx->hmac_ctx == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    ctx->md_info->update_func( ctx->md_ctx, input, ilen );

    return( 0 );
}

int mbedtls_md_hmac_finish( mbedtls_md_context_t *ctx, unsigned char *output )
{
    unsigned char tmp[MBEDTLS_MD_MAX_SIZE];
    unsigned char *opad;

    if( ctx == NULL || ctx->md_info == NULL || ctx->hmac_ctx == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    opad = (unsigned char *) ctx->hmac_ctx + ctx->md_info->block_size;

    ctx->md_info->finish_func( ctx->md_ctx, tmp );
    ctx->md_info->starts_func( ctx->md_ctx );
    ctx->md_info->update_func( ctx->md_ctx, opad, ctx->md_info->block_size );
    ctx->md_info->update_func( ctx->md_ctx, tmp, ctx->md_info->size );
    ctx->md_info->finish_func( ctx->md_ctx, output );

    return( 0 );
}

int mbedtls_md_hmac_reset( mbedtls_md_context_t *ctx )
{
    unsigned char *ipad;

    if( ctx == NULL || ctx->md_info == NULL || ctx->hmac_ctx == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    ipad = (unsigned char *) ctx->hmac_ctx;

    ctx->md_info->starts_func( ctx->md_ctx );
    ctx->md_info->update_func( ctx->md_ctx, ipad, ctx->md_info->block_size );

    return( 0 );
}

int mbedtls_md_hmac( const mbedtls_md_info_t *md_info, const unsigned char *key, size_t keylen,
                const unsigned char *input, size_t ilen,
                unsigned char *output )
{
    mbedtls_md_context_t ctx;
    int ret;

    if( md_info == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    mbedtls_md_init( &ctx );

    if( ( ret = mbedtls_md_setup( &ctx, md_info, 1 ) ) != 0 )
        return( ret );

    mbedtls_md_hmac_starts( &ctx, key, keylen );
    mbedtls_md_hmac_update( &ctx, input, ilen );
    mbedtls_md_hmac_finish( &ctx, output );

    mbedtls_md_free( &ctx );

    return( 0 );
}

int mbedtls_md_process( mbedtls_md_context_t *ctx, const unsigned char *data )
{
    if( ctx == NULL || ctx->md_info == NULL )
        return( MBEDTLS_ERR_MD_BAD_INPUT_DATA );

    ctx->md_info->process_func( ctx->md_ctx, data );

    return( 0 );
}

unsigned char mbedtls_md_get_size( const mbedtls_md_info_t *md_info )
{
    if( md_info == NULL )
        return( 0 );

    return md_info->size;
}

mbedtls_md_type_t mbedtls_md_get_type( const mbedtls_md_info_t *md_info )
{
    if( md_info == NULL )
        return( MBEDTLS_MD_NONE );

    return md_info->type;
}

const char *mbedtls_md_get_name( const mbedtls_md_info_t *md_info )
{
    if( md_info == NULL )
        return( NULL );

    return md_info->name;
}

#endif /* MBEDTLS_MD_C */
