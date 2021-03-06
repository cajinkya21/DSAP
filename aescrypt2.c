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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "platform.h"
#else
#include <stdio.h>
#define mbedtls_fprintf    fprintf
#define mbedtls_printf     printf
#endif

#include "aes.h"
#include "md.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#if !defined(_WIN32_WCE)
#include <io.h>
#endif
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#define MODE_ENCRYPT    0
#define MODE_DECRYPT    1

#define USAGE   \
    "\n  aescrypt2 <mode> <input filename> <output filename> <key>\n" \
    "\n   <mode>: 0 = encrypt, 1 = decrypt\n" \
    "\n  example: aescrypt2 0 file file.aes hex:E76B2413958B00E193\n" \
    "\n  NEW USERS SHOULD READ THE MANUAL ONCE TO UNDERSTAND THE PROPER WORKING (HIGHLY RECOMMOMDED) \n" \

#if !defined(MBEDTLS_AES_C) || !defined(MBEDTLS_SHA256_C) || \
    !defined(MBEDTLS_FS_IO) || !defined(MBEDTLS_MD_C)
int main( void ) {
    mbedtls_printf("MBEDTLS_AES_C and/or MBEDTLS_SHA256_C "
                    "and/or MBEDTLS_FS_IO and/or MBEDTLS_MD_C "
                    "not defined.\n");
    return( 0 );
}
#else
int main( int argc, char *argv[] ) {
    int ret = 1;

    int i, n;
    int mode, lastn;
    size_t keylen;
    FILE *fkey, *fin = NULL, *fout = NULL;

    char *p;
    unsigned char IV[16];
    unsigned char key[512];
    unsigned char digest[32];
    unsigned char buffer[1024];
    unsigned char diff;

    mbedtls_aes_context aes_ctx;
    mbedtls_md_context_t sha_ctx;

#if defined(_WIN32_WCE)
    long filesize, offset;
#elif defined(_WIN32)
       LARGE_INTEGER li_size;
    __int64 filesize, offset;
#else
      off_t filesize, offset;
#endif

    mbedtls_aes_init( &aes_ctx );
    mbedtls_md_init( &sha_ctx );

    if( argc != 5 ) {
        mbedtls_printf( USAGE );

#if defined(_WIN32)
        mbedtls_printf( "\n  Press Enter to exit this program.\n" );
        fflush( stdout ); getchar();
#endif

        goto exit;
    }

    mode = atoi( argv[1] );
    memset(IV, 0, sizeof(IV));
    memset(key, 0, sizeof(key));
    memset(digest, 0, sizeof(digest));
    memset(buffer, 0, sizeof(buffer));

    if( mode != MODE_ENCRYPT && mode != MODE_DECRYPT ) {
        mbedtls_fprintf( stderr, "invalide operation mode\n" );
        goto exit;
    }

    if( strcmp( argv[2], argv[3] ) == 0 ) {
        mbedtls_fprintf( stderr, "input and output filenames must differ\n" );
        goto exit;
    }

    if( ( fin = fopen( argv[2], "rb" ) ) == NULL ) {
        mbedtls_fprintf( stderr, "fopen(%s,rb) failed\n", argv[2] );
        goto exit;
    }

    if( ( fout = fopen( argv[3], "wb+" ) ) == NULL ) {
        mbedtls_fprintf( stderr, "fopen(%s,wb+) failed\n", argv[3] );
        goto exit;
    }

    /*
     * Read the secret key and clean the command line.
     */
    if( ( fkey = fopen( argv[4], "rb" ) ) != NULL ) {
        keylen = fread( key, 1, sizeof( key ), fkey );
        fclose( fkey );
    }
    else {
        if( memcmp( argv[4], "hex:", 4 ) == 0 ) {
            p = &argv[4][4];
            keylen = 0;

            while( sscanf( p, "%02X", &n ) > 0 && keylen < (int) sizeof( key ) ){
                key[keylen++] = (unsigned char) n;
                p += 2;
            }
        }
        else {
            keylen = strlen( argv[4] );

            if( keylen > (int) sizeof( key ) )
                keylen = (int) sizeof( key );

            memcpy( key, argv[4], keylen );
        }
    }

    memset( argv[4], 0, strlen( argv[4] ) );

#if defined(_WIN32_WCE)
    filesize = fseek( fin, 0L, SEEK_END );
#else
#if defined(_WIN32)
    /*
     * Support large files (> 2Gb) on Win32
     */
    li_size.QuadPart = 0;
    li_size.LowPart  =
        SetFilePointer( (HANDLE) _get_osfhandle( _fileno( fin ) ),
                        li_size.LowPart, &li_size.HighPart, FILE_END );

    if( li_size.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR ) {
        mbedtls_fprintf( stderr, "SetFilePointer(0,FILE_END) failed\n" );
        goto exit;
    }

    filesize = li_size.QuadPart;
#else
    if( ( filesize = lseek( fileno( fin ), 0, SEEK_END ) ) < 0 ) {
        perror( "lseek" );
        goto exit;
    }
#endif
#endif

    if( fseek( fin, 0, SEEK_SET ) < 0 ) {
        mbedtls_fprintf( stderr, "fseek(0,SEEK_SET) failed\n" );
        goto exit;
    }

    if( mode == MODE_ENCRYPT ) {
      
        for( i = 0; i < 8; i++ )
            buffer[i] = (unsigned char)( filesize >> ( i << 3 ) );

        p = argv[2];

        mbedtls_md_starts( &sha_ctx );
        mbedtls_md_update( &sha_ctx, buffer, 8 );
        mbedtls_md_update( &sha_ctx, (unsigned char *) p, strlen( p ) );
        mbedtls_md_finish( &sha_ctx, digest );

        memcpy( IV, digest, 16 );

        
        lastn = (int)( filesize & 0x0F );

        IV[15] = (unsigned char)
            ( ( IV[15] & 0xF0 ) | lastn );

        
        if( fwrite( IV, 1, 16, fout ) != 16 ) {
            mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
            goto exit;
        }

        memset( digest, 0,  32 );
        memcpy( digest, IV, 16 );

        for( i = 0; i < 8192; i++ ) {
            mbedtls_md_starts( &sha_ctx );
            mbedtls_md_update( &sha_ctx, digest, 32 );
            mbedtls_md_update( &sha_ctx, key, keylen );
            mbedtls_md_finish( &sha_ctx, digest );
        }

        memset( key, 0, sizeof( key ) );
        mbedtls_aes_setkey_enc( &aes_ctx, digest, 256 );
        mbedtls_md_hmac_starts( &sha_ctx, digest, 32 );

       
        for( offset = 0; offset < filesize; offset += 16 ) {
            n = ( filesize - offset > 16 ) ? 16 : (int)
                ( filesize - offset );

            if( fread( buffer, 1, n, fin ) != (size_t) n ) {
                mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", n );
                goto exit;
            }

            for( i = 0; i < 16; i++ )
                buffer[i] = (unsigned char)( buffer[i] ^ IV[i] );

            mbedtls_aes_crypt_ecb( &aes_ctx, MBEDTLS_AES_ENCRYPT, buffer, buffer );
            mbedtls_md_hmac_update( &sha_ctx, buffer, 16 );

            if( fwrite( buffer, 1, 16, fout ) != 16 ) {
                mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
                goto exit;
            }

            memcpy( IV, buffer, 16 );
        }

        /*
         * Finally write the HMAC.
         */
        mbedtls_md_hmac_finish( &sha_ctx, digest );

        if( fwrite( digest, 1, 32, fout ) != 32 ) {
            mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
            goto exit;
        }
    }

    if( mode == MODE_DECRYPT ) {
        unsigned char tmp[16];
        if( filesize < 48 ) {
            mbedtls_fprintf( stderr, "File too short to be encrypted.\n" );
            goto exit;
        }

        if( ( filesize & 0x0F ) != 0 ) {
            mbedtls_fprintf( stderr, "The file contains plain text so it can not be decrypted \n" );
            goto exit;
        }

        /*
         * Subtract the IV + HMAC length.
         */
        filesize -= ( 16 + 32 );

        /*
         * Read the IV and original filesize modulo 16.
         */
        if( fread( buffer, 1, 16, fin ) != 16 ) {
            mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", 16 );
            goto exit;
        }

        memcpy( IV, buffer, 16 );
        lastn = IV[15] & 0x0F;
        memset( digest, 0,  32 );
        memcpy( digest, IV, 16 );

        for( i = 0; i < 8192; i++ ) {
            mbedtls_md_starts( &sha_ctx );
            mbedtls_md_update( &sha_ctx, digest, 32 );
            mbedtls_md_update( &sha_ctx, key, keylen );
            mbedtls_md_finish( &sha_ctx, digest );
        }

        memset( key, 0, sizeof( key ) );
        mbedtls_aes_setkey_dec( &aes_ctx, digest, 256 );
        mbedtls_md_hmac_starts( &sha_ctx, digest, 32 );

        /*
         * Decrypt and write the plaintext.
         */
        for( offset = 0; offset < filesize; offset += 16 ) {
            if( fread( buffer, 1, 16, fin ) != 16 ) {
                mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", 16 );
                goto exit;
            }

            memcpy( tmp, buffer, 16 );

            mbedtls_md_hmac_update( &sha_ctx, buffer, 16 );
            mbedtls_aes_crypt_ecb( &aes_ctx, MBEDTLS_AES_DECRYPT, buffer, buffer );

            for( i = 0; i < 16; i++ )
                buffer[i] = (unsigned char)( buffer[i] ^ IV[i] );

            memcpy( IV, tmp, 16 );

            n = ( lastn > 0 && offset == filesize - 16 )
                ? lastn : 16;

            if( fwrite( buffer, 1, n, fout ) != (size_t) n ) {
                mbedtls_fprintf( stderr, "fwrite(%d bytes) failed\n", n );
                goto exit;
            }
        }

        /*
         * Verify the message authentication code.
         */
        mbedtls_md_hmac_finish( &sha_ctx, digest );

        if( fread( buffer, 1, 32, fin ) != 32 ) {
            mbedtls_fprintf( stderr, "fread(%d bytes) failed\n", 32 );
            goto exit;
        }

        /* Use constant-time buffer comparison */
        diff = 0;
        for( i = 0; i < 32; i++ )
            diff |= digest[i] ^ buffer[i];

        if( diff != 0 ) {
            mbedtls_fprintf( stderr, "HMAC check failed: wrong key, "
                             "or file corrupted.\n" );
            goto exit;
        }
    }

    ret = 0;

exit:
    if( fin )
        fclose( fin );
    if( fout )
        fclose( fout );

    memset( buffer, 0, sizeof( buffer ) );
    memset( digest, 0, sizeof( digest ) );

    mbedtls_aes_free( &aes_ctx );
    mbedtls_md_free( &sha_ctx );

    return( ret );
}
#endif /* MBEDTLS_AES_C && MBEDTLS_SHA256_C && MBEDTLS_FS_IO */
