#define BUFRW_LIBRARY_BUILD

#include "../include/bufrw.h"

#include <string.h>
#include <stdlib.h>

/* 
   Global static buffers for read and write.
   These buffers will be (re)allocated to the requested size if needed.
*/
static char *read_buffer = NULL;
static int read_buffer_allocated = 0;
static size_t read_buffer_sz = 0;
static size_t read_buffer_pos = 0;   // current position in read_buffer
static size_t read_buffer_len = 0;   // number of valid bytes in read_buffer

static char *write_buffer = NULL;
static int write_buffer_allocated = 0;
static size_t write_buffer_sz = 0;
static size_t write_buffer_pos = 0;  // current position in write_buffer

static bufrw_ver_t bufrwv = { .major=1, .minor=0, .patch=1 };

/*
 * bfver: bufrw version.
 * 
 * Get a current version of libbufrw.
 */
BUFRW_PUBLIC_FUNC bufrw_ver_t bfver() { 
    return bufrwv;
}

/*
 * bfread: buffered fread.
 *
 * Reads up to n elements of size bytes from stream into ptr using an internal buffer
 * of size buffer_sz. Returns the number of complete items read.
 */
BUFRW_PUBLIC_FUNC size_t bfread(void * restrict ptr, size_t size, size_t n, size_t buffer_sz, FILE *restrict stream) {
    // (Re)allocate the read buffer if needed.
    if (read_buffer == NULL || read_buffer_sz != buffer_sz) {
        free(read_buffer);
        read_buffer = (char*)malloc(buffer_sz);
        if (!read_buffer) {
            return 0; // Allocation failed.
        }
        read_buffer_allocated = 1;
        read_buffer_sz = buffer_sz;
        read_buffer_pos = 0;
        read_buffer_len = 0;
    }
    
    size_t total_bytes = size * n;
    size_t bytes_read = 0;
    char *out_ptr = (char *)ptr;
    
    while (bytes_read < total_bytes) {
        // If our buffer is empty, refill it.
        if (read_buffer_pos >= read_buffer_len) {
            read_buffer_len = fread(read_buffer, 1, read_buffer_sz, stream);
            read_buffer_pos = 0;
            if (read_buffer_len == 0) {
                break;  // EOF or read error.
            }
        }
        
        // Determine how many bytes to copy from our internal buffer.
        size_t available = read_buffer_len - read_buffer_pos;
        size_t to_copy = total_bytes - bytes_read;
        if (to_copy > available) {
            to_copy = available;
        }
        
        memcpy(out_ptr + bytes_read, read_buffer + read_buffer_pos, to_copy);
        read_buffer_pos += to_copy;
        bytes_read += to_copy;
    }
    
    return bytes_read / size;  // Return the number of complete items read.
}

/*
 * bfwrite: buffered fwrite.
 *
 * Writes n elements of size bytes from ptr into stream using an internal buffer
 * of size buffer_sz. Returns the number of complete items written.
 */
BUFRW_PUBLIC_FUNC size_t bfwrite(const void * restrict ptr, size_t size, size_t n, size_t buffer_sz, FILE *restrict stream) {
    // (Re)allocate the write buffer if needed.
    if (write_buffer == NULL || write_buffer_sz != buffer_sz) {
        free(write_buffer);
        write_buffer = malloc(buffer_sz);
        if (!write_buffer) {
            return 0;  // Allocation failed.
        }
        write_buffer_allocated = 1;
        write_buffer_sz = buffer_sz;
        write_buffer_pos = 0;
    }
    
    size_t total_bytes = size * n;
    size_t bytes_written = 0;
    const char *in_ptr = (const char *)ptr;
    
    while (bytes_written < total_bytes) {
        size_t available = write_buffer_sz - write_buffer_pos;
        size_t to_copy = total_bytes - bytes_written;
        if (to_copy > available) {
            to_copy = available;
        }
        
        memcpy(write_buffer + write_buffer_pos, in_ptr + bytes_written, to_copy);
        write_buffer_pos += to_copy;
        bytes_written += to_copy;
        
        // If the buffer is full, flush it.
        if (write_buffer_pos == write_buffer_sz) {
            size_t written = fwrite(write_buffer, 1, write_buffer_sz, stream);
            if (written != write_buffer_sz) {
                break;
            }
            write_buffer_pos = 0;
        }
    }
    
    return bytes_written / size;
}

/*
 * bfflush: flush the write buffer.
 *
 * This function writes any data remaining in the internal write buffer to stream.
 */
BUFRW_PUBLIC_FUNC void bfflush(FILE *stream) {
    if (write_buffer && write_buffer_pos > 0) {
        fwrite(write_buffer, 1, write_buffer_pos, stream);
        write_buffer_pos = 0;
    }
}

/*
 * bfseek: buffered fseek.
 *
 * Before performing the actual fseek, flush any pending write data and adjust
 * the read buffer if data has been pre-fetched.
 *
 * Returns 0 on success, or -1 on error.
 */
BUFRW_PUBLIC_FUNC int bfseek(FILE *stream, long offset, int whence) {
    /* If there is any pending write data, flush it first. */
    if (write_buffer && write_buffer_pos > 0) {
        if (fwrite(write_buffer, 1, write_buffer_pos, stream) != write_buffer_pos) {
            return -1;
        }
        write_buffer_pos = 0;
    }
    
    /* If we are reading and the seek is relative to the current position,
       adjust the offset to account for unread bytes in the read buffer.
       (Because the underlying file position is already advanced by the buffered data.) */
    if (read_buffer && whence == SEEK_CUR) {
        offset -= (read_buffer_len - read_buffer_pos);
    }
    
    /* Invalidate the read buffer. */
    read_buffer_pos = 0;
    read_buffer_len = 0;
    
    return fseek(stream, offset, whence);
}

/*
 * bftell: buffered ftell.
 *
 * Returns the "virtual" current position in the stream by adjusting the value
 * returned by ftell() to account for any unread data in the read buffer or any
 * unwritten data in the write buffer.
 */
BUFRW_PUBLIC_FUNC long bftell(FILE *stream) {
    long pos = ftell(stream);
    if (pos == -1L) {
        return -1L;
    }
    
    if (write_buffer && write_buffer_pos > 0) {
        /* If there is pending write data, the effective position is ahead
           of the underlying file position by write_buffer_pos bytes. */
        pos += write_buffer_pos;
    } else if (read_buffer) {
        /* For reads, the underlying file position is ahead of the user’s
           view by the number of unread bytes in the buffer. */
        pos -= (read_buffer_len - read_buffer_pos);
    }
    
    return pos;
}

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
BUFRW_PUBLIC_FUNC size_t bfbestbufsz(size_t fullsz) {
    const size_t MIN_BUFSZ = 512;
    const size_t MAX_BUFSZ = 65536;

    /* If the total size is less than the minimum, use fullsz (unless it is 0) */
    if (fullsz < MIN_BUFSZ)
        return fullsz > 0 ? fullsz : MIN_BUFSZ;

    /* Start at the minimum buffer size and double until limits are reached */
    size_t bufsz = MIN_BUFSZ;
    while (bufsz * 2 <= fullsz && bufsz * 2 <= MAX_BUFSZ)
        bufsz *= 2;

    return bufsz;
}

/*
 * bfcleanup: free the internal buffers.
 *
 * This should be called when you are done using the buffered I/O functions to
 * avoid memory leaks.
 */
BUFRW_DESTRUCTOR
BUFRW_PUBLIC_FUNC void bfcleanup(void) {
    if (read_buffer_allocated  && read_buffer ) {
        free(read_buffer);
        read_buffer_allocated = 0;
    }
    read_buffer_sz = read_buffer_pos = 0;

    if (write_buffer_allocated && write_buffer) {
        free(write_buffer);
        write_buffer_allocated = 0;
    }
    write_buffer_sz = write_buffer_pos = 0;
}