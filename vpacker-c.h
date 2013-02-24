/*-----------------------------------------------------------------------------
 *  vpacker-c.h - A wrapper of vpacker32/64 for C
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2013 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

#ifndef __INCLUDE_VPACKER_H__
#define __INCLUDE_VPACKER_H__

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------
 * A interface for compression, a input 32-bit or
 * 64-bit array in *src is compressed, and output
 * to *dst as a byte sequence.
 *
 * src    : input buffer
 * dst    : output buffer
 * n      : # of input integers
 * return : # of written bytes for compression
 *-------------------------------------------------
 */
extern size_t vpacker32_compress(const uint32_t *src,
                                 char *dst,
                                 size_t n);

extern size_t vpacker64_compress(const uint64_t *src,
                                 char *dst,
                                 size_t n);


/*-------------------------------------------------
 * A interface for decompression, a input byte
 * sequence compressed by vpacker32/64_compress()
 * is decompressed, and output to *dst as a 32-bit
 * or 64-bit array.
 *
 * src    : input buffer
 * dst    : output buffer
 * n      : # of input bytes
 * return : # of read bytes for decompression
 *-------------------------------------------------
 */
extern size_t vpacker32_uncompress(const char *src,
                                   uint32_t *dst,
                                   size_t n);



extern size_t vpacker64_uncompress(const char *src,
                                   uint64_t *dst,
                                   size_t n);


/*-------------------------------------------------
 * The function provides the maximumx size that
 * vpacker32/64_compress() may output. It is useful
 * to know the size in advance because of memory
 * allocation for *dst during compression.
 *
 *  n      : # of input integers
 *  return : maximum size of compressed data
 *-------------------------------------------------
 */
extern size_t vpacker32_compress_bound(size_t n);
extern size_t vpacker64_compress_bound(size_t n);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __INCLUDE_VPACKER_H__ */
