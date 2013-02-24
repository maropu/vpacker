/*-----------------------------------------------------------------------------
 *  libvpack.cpp - A wrapper of vpacker32/64 for C
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2013 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

#include <vpacker-c.h>

#include <vpacker32.hpp>
#include <vpacker64.hpp>

/* Compression for a 32/64-bit array */
size_t vpacker32_compress(
    const uint32_t *src, char *dst, size_t n) {
  return vpacker32::Compress(src, dst, n);
}

size_t vpacker64_compress(
    const uint64_t *src, char *dst, size_t n) {
  return vpacker64::Compress(src, dst, n);
}


/* Decompression for a 32/64-bit array */
size_t vpacker32_uncompress(
    const char *src, uint32_t *dst, size_t n) {
  return vpacker32::Uncompress(src, dst, n);
}

size_t vpacker64_uncompress(
    const char *src, uint64_t *dst, size_t n) {
  return vpacker64::Uncompress(src, dst, n);
}


/* helper functions for compression */
size_t vpacker32_compress_bound(size_t n) {
  return vpacker32::CompressBound(n);
}

size_t vpacker64_compress_bound(size_t n) {
  return vpacker64::CompressBound(n);
}
