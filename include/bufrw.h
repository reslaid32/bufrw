/*
 * bufrw.h - Buffered Read and Write I/O Operations
 *
 * Project: bufrw
 * License: MIT
 * Author: [reslaid32]
 *
 * Description:
 * This library provides optimized buffered read and write operations for
 * file streams, improving performance by reducing direct system calls.
 * It includes functions for reading, writing, seeking, and managing
 * buffer sizes efficiently.
 *
 * License:
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef BUFRW_H
#define BUFRW_H

#include <stdio.h>
#include <stddef.h>

// virtual export.h
#if !defined(BUFRW_EXPORT_H_LOADED)
#define BUFRW_EXPORT_H_LOADED

#if !defined(BUFRW_DEPRECATED)
#define BUFRW_DEPRECATED(msg) __attribute__((deprecated(msg)))
#endif // BUFRW_DEPRECATED

#ifdef BUFRW_WITH_EXECUTABLE
#define BUFRW_EXPORT
#else
#if defined(_WIN32) || defined(WIN32)
#ifdef BUFRW_LIBRARY_BUILD
#define BUFRW_EXPORT __declspec(dllexport)
#else
#define BUFRW_EXPORT __declspec(dllimport)
#endif
#else
#ifdef BUFRW_LIBRARY_BUILD
#define BUFRW_EXPORT __attribute__((visibility("default")))
#else
#define BUFRW_EXPORT
#endif
#endif
#endif

#if !defined(BUFRW_PUBLIC_FUNC)
#define BUFRW_PUBLIC_FUNC BUFRW_EXPORT
#endif // BUFRW_PUBLIC_FUNC

#if !defined(BUFRW_PRIVATE_FUNC)
    #if defined(_MSC_VER)
        #define BUFRW_PRIVATE_FUNC static __forceinline
    #elif defined(__GNUC__) || defined(__clang__)
        #define BUFRW_PRIVATE_FUNC static __inline__ __attribute__((always_inline))
    #else
        #define BUFRW_PRIVATE_FUNC static inline
    #endif
#endif // BUFRW_PRIVATE_FUNC

#if !defined(BUFRW_PUBLIC_HO_FUNC)
/* public header only function */
#define BUFRW_PUBLIC_HO_FUNC BUFRW_PRIVATE_FUNC
#endif // BUFRW_PUBLIC_HO_FUNC

#if !defined(BUFRW_EXTERN_C)
#if defined(__cplusplus)
#define BUFRW_EXTERN_C extern "C"
#else
#define BUFRW_EXTERN_C
#endif // __cplusplus
#endif // BUFRW_EXTERN_C

#if !defined(BUFRW_EXTERN_C_BEG)
#if defined(__cplusplus)
#define BUFRW_EXTERN_C_BEG BUFRW_EXTERN_C {
#else
#define BUFRW_EXTERN_C_BEG
#endif // __cplusplus
#endif // BUFRW_EXTERN_C_BEG

#if !defined(BUFRW_EXTERN_C_END)
#if defined(__cplusplus)
#define BUFRW_EXTERN_C_END }
#else
#define BUFRW_EXTERN_C_END
#endif // __cplusplus
#endif // BUFRW_EXTERN_C_END

#if !defined(BUFRW_CONSTRUCTOR)
#define BUFRW_CONSTRUCTOR __attribute__((constructor))
#endif /* BUFRW_CONSTRUCTOR */

#if !defined(BUFRW_DESTRUCTOR)
#define BUFRW_DESTRUCTOR __attribute__((destructor))
#endif /* BUFRW_DESTRUCTOR */

#endif // BUFRW_EXPORT_H_LOADED

// virtual version.h
#if !defined(BUFRW_VERSION_H_LOADED)
#define BUFRW_VERSION_H_LOADED

typedef struct _s_bufrw_ver {
    unsigned major, minor, patch;
} bufrw_ver_t;

/*
 * bfver: bufrw version.
 * 
 * Get a current version of libbufrw.
 */
BUFRW_PUBLIC_FUNC bufrw_ver_t bfver(void);

#endif // BUFRW_VERSION_H_LOADED

/*
 * bfread: buffered fread.
 *
 * Reads up to n elements of size bytes from stream into ptr using an internal buffer
 * of size buffer_sz. Returns the number of complete items read.
 */
BUFRW_PUBLIC_FUNC size_t bfread(void *ptr, size_t size, size_t n, size_t buffer_sz, FILE *stream);

/*
 * bfwrite: buffered fwrite.
 *
 * Writes n elements of size bytes from ptr into stream using an internal buffer
 * of size buffer_sz. Returns the number of complete items written.
 */
BUFRW_PUBLIC_FUNC size_t bfwrite(const void *ptr, size_t size, size_t n, size_t buffer_sz, FILE *stream);

/*
 * bfflush: flush the write buffer.
 *
 * This function writes any data remaining in the internal write buffer to stream.
 */
BUFRW_PUBLIC_FUNC void bfflush(FILE *stream);

/*
 * bfseek: buffered fseek.
 *
 * Before performing the actual fseek, flush any pending write data and adjust
 * the read buffer if data has been pre-fetched.
 *
 * Returns 0 on success, or -1 on error.
 */
BUFRW_PUBLIC_FUNC int bfseek(FILE *stream, long offset, int whence);

/*
 * bftell: buffered ftell.
 *
 * Returns the "virtual" current position in the stream by adjusting the value
 * returned by ftell() to account for any unread data in the read buffer or any
 * unwritten data in the write buffer.
 */
BUFRW_PUBLIC_FUNC long bftell(FILE *stream);

/*
 * bfbestbufsz - Choose an optimal buffer size based on the total size.
 *
 * This function calculates a recommended buffer size (bufsz) given the total amount
 * of data (fullsz) that needs to be processed. It starts at a defined minimum buffer size
 * and doubles it (choosing power-of-two sizes) until further doubling would exceed either
 * the full size or a defined maximum buffer size.
 *
 * Parameters:
 *   fullsz - The total size of the data to be processed.
 *
 * Returns:
 *   A buffer size (bufsz) that is at least MIN_BUFSZ, does not exceed MAX_BUFSZ,
 *   and is the largest power-of-two not greater than fullsz (when fullsz is larger than MIN_BUFSZ).
 */
BUFRW_PUBLIC_FUNC size_t bfbestbufsz(size_t fullsz);

/*
 * bfcleanup: free the internal buffers.
 *
 * This should be called when you are done using the buffered I/O functions to
 * avoid memory leaks.
 */
BUFRW_DESTRUCTOR
BUFRW_PUBLIC_FUNC void bfcleanup(void);

#endif // BUFRW_H