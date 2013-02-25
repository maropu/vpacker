/*-----------------------------------------------------------------------------
 *  vpacker64.hpp - A simple encoder/decoder library for 64-bit integers
 *    The codes use a paper below as a refererence.
 *     http://dl.acm.org/citation.cfm?id=1871592
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2013 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

#ifndef __INCLUDE_VPACKER64_HPP__
#define __INCLUDE_VPACKER64_HPP__

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

/* A C99 standard option */
#if __STDC_VERSION__ < 199901L
# define restrict
#endif

/*
 * Some compilers already have a definition,
 * so it is undefined first and defined again.
 */
#ifdef ARRAYSIZE
# undef ARRAYSIZE
#endif

#define ARRAYSIZE(__x__)    \
    (sizeof(__x__) / sizeof(*(__x__)))

/* An assertion macro */
#ifndef NDEBUG
# define VP64_ASSERT(__x__)   assert(__x__)
#else
# define VP64_ASSERT(__x__)
#endif

/* Hardware bit-count detection */
#if defined(__GNUC__)
# define VP64_MSB64(__x__)  \
    ((__x__ != 0)? __builtin_clzll(__x__) : 64)
#else
/* A portable code for bit counting */
inline int VP64_MSB64(uint64_t x){
  int pos = 64;

  /* Bits decrease logarithmically */
  uint64_t temp = x >> 32;
  if (temp != 0) {pos -= 32; x = temp;}
  temp = x >> 16;
  if (temp != 0) {pos -= 16; x = temp;}
  temp = x >> 8;
  if (temp != 0) {pos -= 8; x = temp;}
  temp = x >> 4;
  if (temp != 0) {pos -= 4; x = temp;}
  temp = x >> 2;
  if (temp != 0) {pos -= 2; x = temp;}
  temp = x >> 1;
  if (temp != 0) {return pos - 2;}

  VP64_ASSERT(pos > static_cast<int>(x));
  return pos - static_cast<int>(x);
}
#endif

#define VP64_DIV_ROUNDUP(__x__, __y__)  \
    ((__x__ + __y__ - 1) / __y__)

namespace vpacker64 {

/*
 * VP64_MAGICNUM was picked by running
 *    cat vpacker64.hpp | sha1sum
 * and taking the leading 64 bits.
 */
static const uint64_t VP64_MAGICNUM = 0x08b5a7033f4cbc3dULL;

namespace backend {

/*
 * Integers in each partition are packed with
 * pre-defined lengths below.
 */
static const int bits_length[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 16, 32, 64
};

/*
 * Round-up bit lengths from the actual ones
 * to corresponding bits_length[].
 */
static const int roundup_bits[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 16, 16, 16, 16,
  32, 32, 32, 32, 32, 32, 32, 32,
  32, 32, 32, 32, 32, 32, 32, 32,
  64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64,
  64, 64, 64, 64, 64, 64, 64, 64
};

/*
 * Partition lengths split by using Dynamic
 * Programming in ComputePartition().
 */
static const size_t partition_length[] = {
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 16, 32, 64, 128
};

/*
 * A control byte is packed together with
 * compressed data to decide which unpacker
 * is used for UncompressBlock() and how many
 * bytes are consumed for the unpacker.
 * Two look-up tables below are used to map
 * actual values in bits_length[] and
 * partition_length[] into these indices.
 */
static const char ctrl_bit[] = {
  0x00, 0x01, 0x02, 0x03,
  0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b,
  0x0c, 0xff, 0xff, 0xff,
  0x0d, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0x0e, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0x0f
};

static const char ctrl_partition[] = {
  0xff, 0x00, 0x10, 0x20,
  0x30, 0x40, 0x50, 0x60,
  0x70, 0x80, 0x90, 0xa0,
  0xb0, 0xff, 0xff, 0xff,
  0xc0, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xd0, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xe0, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff,
  0xf0
};

/*
 * A input sequence of integers is split into
 * block_num ones, and the integers are compressed
 * together with CompressBlock().
 */
static const size_t block_num = 65536;


/*-------------------------------------------------
 * A writer/reader for a given unsigned value
 * while taking into accounts byte-ordering.
 * NOTE: A caller must use following functions
 * without any exception.
 *
 * SetUint32/64
 *  out : output buffer
 *  v   : 32-bit unsigned value to write
 *
 * DecodeUint16/32/64
 *  in     : input packed bytes
 *  return : 16/32/64-bit value to read
 *-------------------------------------------------
 */
inline void SetUint32(char *restrict out,
                      uint32_t v) {
  out[0] = (v >> 24) & 0xff;
  out[1] = (v >> 16) & 0xff;
  out[2] = (v >> 8) & 0xff;
  out[3] = v & 0xff;
}

inline void SetUint64(char *restrict out,
                      uint64_t v) {
  out[0] = (v >> 56) & 0xff;
  out[1] = (v >> 48) & 0xff;
  out[2] = (v >> 40) & 0xff;
  out[3] = (v >> 32) & 0xff;
  out[4] = (v >> 24) & 0xff;
  out[5] = (v >> 16) & 0xff;
  out[6] = (v >> 8) & 0xff;
  out[7] = v & 0xff;
}

inline uint16_t
    DecodeUint16(const char *restrict in) {
  uint16_t v = in[0] & 0xff;
  v = (v << 8) | (in[1] & 0xff);
  return v;
}

inline uint32_t
    DecodeUint32(const char *restrict in) {
  uint32_t v = in[0] & 0xff;
  for (int i = 1; i < 4; i++)
    v = (v << 8) | (in[i] & 0xff);
  return v;
}

inline uint64_t
    DecodeUint64(const char *restrict in) {
  uint64_t v = in[0] & 0xff;
  for (int i = 1; i < 8; i++)
    v = (v << 8) | (in[i] & 0xff);
  return v;
}


/*-------------------------------------------------
 * A writer function with fixed-length bits while
 * using SetUint32(). It buffers input data to
 * write, and flush them by each 32-bit value.
 *
 *  src    : integer array to write
 *  nbits  : # of written bits
 *  n      : # of input integers
 *  out    : output bffuer
 *  limit  : terminal address of *out
 *  return : # of written bytes, or -1 if it fails
 *-------------------------------------------------
 */
inline int WriteBits(const uint64_t *src,
                     int nbits,
                     size_t n,
                     char *dst,
                     const char *restrict dlimit) {
  VP64_ASSERT(src != NULL);
  VP64_ASSERT(nbits >= 0 && nbits <= 64);
  VP64_ASSERT(n <= 128);
  VP64_ASSERT(dst != NULL);
  VP64_ASSERT(dlimit != NULL);

  /*
   * Calculate the number of bytes which will
   * be used in the functions.
   */
  int nwritten = VP64_DIV_ROUNDUP(nbits * n, 8);

  if (dst + nwritten > dlimit)
    return -1;

  /* If nbits == 0, do nothing */
  if (nbits == 0)
    return 0;

  /* If nbits == 64, just copy them */
  if (nbits == 64) {
    for (size_t i = 0; i < n; i++)
      SetUint64(dst + i * 8, src[i]);
    return n * 8;
  }

  /*
   * Otherwise, buffer written bits in a 64-bit
   * value (buf), and write them.
   */
  int       nused = 0;
  uint64_t  buf = 0;

  for (size_t i = 0; i < n; i++) {
    buf = (buf << nbits) |
        (src[i] & ((uint64_t(1) << nbits) - 1));
    nused += nbits;

    if (nused >= 32) {
      uint32_t w = (buf >> (nused - 32)) &
          ((uint64_t(1) << 32) - 1);
      SetUint32(dst, w);
      nused -= 32;
      dst += 4;
    }
  }

  /* If any, flush left bits */
  if (nused > 0) {
    uint32_t w = (buf << (32 - nused)) &
        ((uint64_t(1) << 32) - 1);

    int nc = VP64_DIV_ROUNDUP(nused, 8);
    VP64_ASSERT(nc != 0);

    switch (nc) {
      case 4: {dst[3] = w & 0xff;}
      case 3: {dst[2] = (w >> 8) & 0xff;}
      case 2: {dst[1] = (w >> 16) & 0xff;}
      case 1: {dst[0] = (w >> 24) & 0xff;}
    }
  }

  return nwritten;
}


/*-------------------------------------------------
 * A function computes optimal partitions to
 * pack integers by Dynamic Programming.
 *
 *  src    : integer array to partition with DP
 *  n      : # of input integers
 *  parts  : result partitions
 *  return : # of partitions
 *-------------------------------------------------
 */
inline int ComputePartition(const uint64_t *src,
                            size_t n,
                            size_t *parts) {
  VP64_ASSERT(src != NULL);
  VP64_ASSERT(parts != NULL);

  const size_t max_partition =
      partition_length[ARRAYSIZE(partition_length) - 1];

  VP64_ASSERT(n >= max_partition);

  /*
   * refs[] stores backward references to partition
   * *src. refs[i] - refs[i-1] is a length of calculated
   * partitions, and costs[i] stores cost values
   * corresponding to the partitoin. Initially, refs[]
   * and costs[] are set to -1 and 0.
   */
  int64_t   refs[n + 1];
  uint64_t  costs[n + 1];

  for (size_t i = 0; i <= n; i++) {
    refs[i] = -1;
    costs[i] = 0;
  }

  /*
   * Initialize costs in costs[0...max_partition-1]
   * Leading max_partition-elements in refs[] must
   * reference to the previous one there.
   */
  costs[0] = VP64_DIV_ROUNDUP(
      64 - VP64_MSB64(src[0]), 8);
  for (size_t i = 1;
        i < max_partition; i++) {
    refs[i] = i - 1;
    costs[i] = costs[i - 1] +
        VP64_DIV_ROUNDUP(64 - VP64_MSB64(src[i]), 8);
  }

  for (size_t i = max_partition; i <= n; i++) {
    int     maxb = 0;

    for (size_t j = 0;
          j < ARRAYSIZE(partition_length); j++) {
      size_t bp = i - partition_length[j];

      /*
       * Update a maximum bit length in
       * a given array.
       */
      int b = roundup_bits[
          64 - VP64_MSB64(src[bp])];
      if (maxb < b)
        maxb = b;

      uint64_t c = costs[bp] +
          VP64_DIV_ROUNDUP((i - bp) * maxb, 8);

      if (refs[i] == -1 || c <= costs[i]) {
        costs[i] = c;
        refs[i] = bp;
      }
    }
  }

  /* Compute the number of partitions */
  int     pnum = 0;
  size_t  next = n;

  while (next != 0) {next = refs[next]; pnum++;}

  /* Give back optimal partitons to a caller */
  int     pidx = pnum;

  while (n != 0) {parts[pidx--] = n; n = refs[n];}
  parts[0] = 0;

  return pnum;
}


/*-------------------------------------------------
 * Unpack fixed-bit integers by a given length.
 * NOTE: The following functions overrun a given
 * buffer to some extent in view of unpacking
 * performance. Therefore, a caller must use the
 * functions while taking into accounts the
 * overruns during unpacking.
 *
 *  src    : integer array to pack
 *  slimit : terminal address of *src
 *  dst    : output buffer
 *  dlimit : terminal address of *dst
 *  n      : # of decompressed integers
 *  return : # of read bytes, or -1 if it fails
 *
 * XXX: Unpacking integers byte-by-byte eats many
 * processor time, so it is better to exploit
 * 32-bit or 64-bit registers; load consecutive
 * bytes in *src into the registers, and scatter
 * into a output buffer in *dst, while taking into
 * accounts byte-ordering and unaligned loads.
 *-------------------------------------------------
 */

/*
 * Currently, Unpack9 overruns *dst by 128 bytes.
 * So, last sixteen 64-bit integers are left as
 * it is during compression, that is, just copied
 * to a output buffer.
 */
const size_t MAX_UNPACK_OVERRUN_NUM = 16;

inline int Unpack0(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  /* Needed to suppress some warnings */
  VP64_ASSERT(src != NULL);
  VP64_ASSERT(slimit != NULL);

  if (dst + n > dlimit)
    return -1;

  memset(dst, 0x00, n * 8);
  return 0;
}

inline int Unpack1(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 8);
  if (src + nloop > slimit ||
        dst + 8 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop ; i++) {
    for (int j = 0; j < 8; j++)
      dst[j] = (src[0] >> (7 - j)) & 0x01;

    src += 1;
    dst += 8;
  }

  return nloop;
}

inline int Unpack2(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 4);
  if (src + nloop > slimit ||
        dst + 4 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    for (int j = 0; j < 4; j++)
      dst[j] = (src[0] >> (6 - 2 * j)) & 0x03;

    src += 1;
    dst += 4;
  }

  return nloop;
}

inline int Unpack3(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 8);
  if (src + 3 * nloop > slimit ||
        dst + 8 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    dst[0] = (src[0] >> 5) & 0x07;
    dst[1] = (src[0] >> 2) & 0x07;
    dst[2] = (src[0] << 1) & 0x07;
    /*
     * A '0x01' mask must be needed because
     * of arithmetic shifts for singed
     * types, or *src, in gcc and other
     * typical compilers.
     */
    dst[2] |= (src[1] >> 7) & 0x01;
    dst[3] = (src[1] >> 4) & 0x07;
    dst[4] = (src[1] >> 1) & 0x07;
    dst[5] = (src[1] << 2) & 0x07;
    dst[5] |= (src[2] >> 6) & 0x03;
    dst[6] = (src[2] >> 3) & 0x07;
    dst[7] = src[2] & 0x07;

    src += 3;
    dst += 8;
  }

  return VP64_DIV_ROUNDUP(3 * n, 8);
}

inline int Unpack4(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 2);
  if (src + nloop > slimit ||
        dst + 2 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    for (int j = 0; j < 2; j++)
      dst[j] = (src[0] >> (4 - 4 * j)) & 0x0f;

    src += 1;
    dst += 2;
  }

  return nloop;
}

inline int Unpack5(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 8);
  if (src + 5 * nloop > slimit ||
        dst + 8 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    dst[0] = (src[0] >> 3) & 0x1f;
    dst[1] = (src[0] << 2) & 0x1f;
    dst[1] |= (src[1] >> 6) & 0x03;
    dst[2] = (src[1] >> 1) & 0x1f;
    dst[3] = (src[1] << 4) & 0x1f;
    dst[3] |= (src[2] >> 4) & 0x0f;
    dst[4] = (src[2] << 1) & 0x1f;
    dst[4] |= (src[3] >> 7) & 0x01;
    dst[5] = (src[3] >> 2) & 0x1f;
    dst[6] = (src[3] << 3) & 0x1f;
    dst[6] |= (src[4] >> 5) & 0x07;
    dst[7] = src[4] & 0x1f;

    src += 5;
    dst += 8;
  }

  return VP64_DIV_ROUNDUP(5 * n, 8);
}

inline int Unpack6(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 4);
  if (src + 3 * nloop > slimit ||
        dst + 4 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    dst[0] = (src[0] >> 2) & 0x3f;
    dst[1] = (src[0] << 4) & 0x3f;
    dst[1] |= (src[1] >> 4) & 0x0f;
    dst[2] = (src[1] << 2) & 0x3f;
    dst[2] |= (src[2] >> 6) & 0x03;
    dst[3] = src[2] & 0x3f;

    src += 3;
    dst += 4;
  }

  return VP64_DIV_ROUNDUP(3 * n, 4);
}

inline int Unpack7(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 8);
  if (src + 7 * nloop > slimit ||
        dst + 8 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    dst[0] = (src[0] >> 1) & 0x7f;
    dst[1] = (src[0] << 6) & 0x7f;
    dst[1] |= (src[1] >> 2) & 0x3f;
    dst[2] = (src[1] << 5) & 0x7f;
    dst[2] |= (src[2] >> 3) & 0x1f;
    dst[3] = (src[2] << 4) & 0x7f;
    dst[3] |= (src[3] >> 4) & 0x0f;
    dst[4] = (src[3] << 3) & 0x7f;
    dst[4] |= (src[4] >> 5) & 0x07;
    dst[5] = (src[4] << 2) & 0x7f;
    dst[5] |= (src[5] >> 6) & 0x03;
    dst[6] = (src[5] << 1) & 0x7f;
    dst[6] |= (src[6] >> 7) & 0x01;
    dst[7] = src[6] & 0x7f;

    src += 7;
    dst += 8;
  }

  return VP64_DIV_ROUNDUP(7 * n, 8);
}

inline int Unpack8(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  if (src + n > slimit || dst + n > dlimit)
    return -1;

  for (int i = 0; i < n; i++) {
    dst[0] = src[0] & 0xff;

    src += 1;
    dst += 1;
  }

  return n;
}

inline int Unpack9(const char *restrict src,
                   const char *restrict slimit,
                   uint64_t *restrict dst,
                   const uint64_t *restrict dlimit,
                   int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 16);
  if (src + 18 * nloop > slimit ||
        dst + 16 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    /*
     * At first, Merge a input sequence
     * into 2-byte chunks.
     */
    uint16_t v[9];

    for (int j = 0; j < 9; j++) {
      v[j] = DecodeUint16(src);
      src += 2;
    }

    /* Then, scatter into a output */
    dst[0] = (v[0] >> 7) & 0x01ff;
    dst[1] = (v[0] << 2) & 0x01ff;
    dst[1] |= v[1] >> 14;
    dst[2] = (v[1] >> 5) & 0x01ff;
    dst[3] = (v[1] << 4) & 0x01ff;
    dst[3] |= v[2] >> 12;
    dst[4] = (v[2] >> 3) & 0x01ff;
    dst[5] = (v[2] << 6) & 0x01ff;
    dst[5] |= v[3] >> 10;
    dst[6] = (v[3] >> 1) & 0x01ff;
    dst[7] = (v[3] << 8) & 0x01ff;
    dst[7] |= v[4] >> 8;
    dst[8] = (v[4] << 1) & 0x01ff;
    dst[8] |= v[5] >> 15;
    dst[9] = (v[5] >> 6) & 0x01ff;
    dst[10] = (v[5] << 3) & 0x01ff;
    dst[10] |= v[6] >> 13;
    dst[11] = (v[6] >> 4) & 0x01ff;
    dst[12] = (v[6] << 5) & 0x01ff;
    dst[12] |= v[7] >> 11;
    dst[13] = (v[7] >> 2) & 0x01ff;
    dst[14] = (v[7] << 7) & 0x01ff;
    dst[14] |= v[8] >> 9;
    dst[15] = v[8] & 0x01ff;

    dst += 16;
  }

  return VP64_DIV_ROUNDUP(9 * n, 8);
}

inline int Unpack10(const char *restrict src,
                    const char *restrict slimit,
                    uint64_t *restrict dst,
                    const uint64_t *restrict dlimit,
                    int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 8);
  if (src + 10 * nloop > slimit ||
        dst + 8 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    uint16_t v[5];

    for (int j = 0; j < 5; j++) {
      v[j] = DecodeUint16(src);
      src += 2;
    }

    /* Then, scatter into a output */
    dst[0] = (v[0] >> 6) & 0x03ff;
    dst[1] = (v[0] << 4) & 0x03ff;
    dst[1] |= v[1] >> 12;
    dst[2] = (v[1] >> 2) & 0x03ff;
    dst[3] = (v[1] << 8) & 0x03ff;
    dst[3] |= v[2] >> 8;
    dst[4] = (v[2] << 2) & 0x03ff;
    dst[4] |= v[3] >> 14;
    dst[5] = (v[3] >> 4) & 0x03ff;
    dst[6] = (v[3] << 6) & 0x03ff;
    dst[6] |= v[4] >> 10;
    dst[7] = v[4] & 0x03ff;

    dst += 8;
  }

  return VP64_DIV_ROUNDUP(5 * n, 4);
}

inline int Unpack11(const char *restrict src,
                    const char *restrict slimit,
                    uint64_t *restrict dst,
                    const uint64_t *restrict dlimit,
                    int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 16);
  if (src + 22 * nloop > slimit ||
        dst + 16 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    uint16_t v[11];

    for (int j = 0; j < 11; j++) {
      v[j] = DecodeUint16(src);
      src += 2;
    }

    /* Then, scatter into a output */
    dst[0] = (v[0] >> 5) & 0x07ff;
    dst[1] = (v[0] << 6) & 0x07ff;
    dst[1] |= v[1] >> 10;
    dst[2] = (v[1] << 1) & 0x07ff;
    dst[2] |= v[2] >> 15;
    dst[3] = (v[2] >> 4) & 0x07ff;
    dst[4] = (v[2] << 7) & 0x07ff;
    dst[4] |= v[3] >> 9;
    dst[5] = (v[3] << 2) & 0x07ff;
    dst[5] |= v[4] >> 14;
    dst[6] = (v[4] >> 3) & 0x07ff;
    dst[7] = (v[4] << 8) & 0x07ff;
    dst[7] |= v[5] >> 8;
    dst[8] = (v[5] << 3) & 0x07ff;
    dst[8] |= v[6] >> 13;
    dst[9] = (v[6] >> 2) & 0x07ff;
    dst[10] = (v[6] << 9) & 0x07ff;
    dst[10] |= v[7] >> 7;
    dst[11] = (v[7] << 4) & 0x07ff;
    dst[11] |= v[8] >> 12;
    dst[12] = (v[8] >> 1) & 0x07ff;
    dst[13] = (v[8] << 10) & 0x07ff;
    dst[13] |= v[9] >> 6;
    dst[14] = (v[9] << 5) & 0x07ff;
    dst[14] |= v[10] >> 11;
    dst[15] = v[10] & 0x07ff;

    dst += 16;
  }

  return VP64_DIV_ROUNDUP(11 * n, 8);
}

inline int Unpack12(const char *restrict src,
                    const char *restrict slimit,
                    uint64_t *restrict dst,
                    const uint64_t *restrict dlimit,
                    int n) {
  int nloop = VP64_DIV_ROUNDUP(n, 4);
  if (src + 6 * nloop > slimit ||
        dst + 4 * nloop > dlimit)
    return -1;

  for (int i = 0; i < nloop; i++) {
    uint16_t v[3];

    for (int j = 0; j < 3; j++) {
      v[j] = DecodeUint16(src);
      src += 2;
    }

    /* Then, scatter into a output */
    dst[0] = (v[0] >> 4) & 0x0fff;
    dst[1] = (v[0] << 8) & 0x0fff;
    dst[1] |= v[1] >> 8;
    dst[2] = (v[1] << 4) & 0x0fff;
    dst[2] |= v[2] >> 12;
    dst[3] = v[2] & 0x0fff;

    dst += 4;
  }

  return VP64_DIV_ROUNDUP(3 * n, 2);
}

inline int Unpack16(const char *restrict src,
                    const char *restrict slimit,
                    uint64_t *restrict dst,
                    const uint64_t *restrict dlimit,
                    int n) {
  if (src + 2 * n > slimit || dst + n > dlimit)
    return -1;

  for (int i = 0; i < n; i++) {
    dst[0] = DecodeUint16(src);

    src += 2;
    dst += 1;
  }

  return 2 * n;
}

inline int Unpack32(const char *restrict src,
                    const char *restrict slimit,
                    uint64_t *restrict dst,
                    const uint64_t *restrict dlimit,
                    int n) {
  if (src + 4 * n > slimit || dst + n > dlimit)
    return -1;

  for (int i = 0; i < n; i++) {
    dst[0] = DecodeUint32(src);

    src += 4;
    dst += 1;
  }

  return 4 * n;
}

inline int Unpack64(const char *restrict src,
                    const char *restrict slimit,
                    uint64_t *restrict dst,
                    const uint64_t *restrict dlimit,
                    int n) {
  if (src + 8 * n > slimit || dst + n > dlimit)
    return -1;

  for (int i = 0; i < n; i++) {
    dst[0] = DecodeUint64(src);

    src += 8;
    dst += 1;
  }

  return 8 * n;
}


/*-------------------------------------------------
 * Following functions are to help the
 * implementations of Compress() and Uncompress().
 *
 * CompressBlock
 *  src    : integer array to compress
 *  n      : # of input integers
 *  dst    : output buffer
 *  dlimit : terminal address of *dst
 *  return : # of written bytes, or 0 if it fails
 *-------------------------------------------------
 */
inline uint32_t CompressBlock(const uint64_t *src,
                              size_t n,
                              char *dst,
                              const char *restrict dlimit) {
  VP64_ASSERT(src != NULL);
  VP64_ASSERT(dst != NULL);
  VP64_ASSERT(n != 0);

  const size_t max_partition =
      partition_length[ARRAYSIZE(partition_length) - 1];

  if (max_partition +
        MAX_UNPACK_OVERRUN_NUM > n) {
    for (size_t i = 0; i < n; i++)
      SetUint64(dst + 8 * i, src[i]);

    return n * 8;
  }

  /*
   * Last MAX_UNPACK_OVERRUN_NUM 64-bit integers
   * are not compressed and just copied to a output
   * buffer because of the overrun issue during
   * decompression.
   */
  n -= MAX_UNPACK_OVERRUN_NUM;

  /* parts[] uses stack space */
  size_t parts[n + 1];

  int np = ComputePartition(src, n, parts);

  uint32_t offset = np + 8;
  SetUint32(dst + 4, offset);

  char *ctrl = dst + 8;
  char *data = dst + offset;

  /* Do compressing */
  uint32_t block_size = offset;

  for (int i = 0; i < np; i++) {
    size_t plen =
        parts[i + 1] - parts[i];

    int maxb = 0;
    for (size_t j = 0; j < plen; j++) {
      int b = roundup_bits[
          64 - VP64_MSB64(src[j])];
      if (maxb < b)
        maxb = b;
    }

    int nwrite = WriteBits(
        src, maxb, plen, data, dlimit);

    /* Check if it works correctly */
    if (nwrite < 0)
      return 0;

    /* Write a control byte */
    *ctrl = ctrl_bit[maxb] | ctrl_partition[plen];

    VP64_ASSERT(ctrl_bit[maxb] != char(0xff));
    VP64_ASSERT(ctrl_partition[plen] != char(0xff));

    /* Move to a next partition */
    src += plen;
    data += nwrite;
    ctrl++;
    block_size += nwrite;
  }

  /* Copy left integers to a output */
  for (size_t i = 0;
        i < MAX_UNPACK_OVERRUN_NUM; i++)
    SetUint64(data + i * 8, src[i]);

  block_size += 8 * MAX_UNPACK_OVERRUN_NUM;

  /*
   * Finally, it stores the size of
   * this block in the leading 4-byte
   * space of the block.
   */
  SetUint32(dst, block_size);

  return block_size;
}


/*-------------------------------------------------
 * Following functions are to help the
 * implementations of Compress() and Uncompress().
 *
 * UncompressBlock
 *  src    : sequence of compressed bytes
 *  dst    : output buffer
 *  n      : # of decompressed integers
 *  return : # of read bytes, or 0 if it fails
 *-------------------------------------------------
 */
inline uint32_t UncompressBlock(const char *src,
                                uint64_t *dst,
                                size_t n) {
  VP64_ASSERT(src != NULL);
  VP64_ASSERT(dst != NULL);
  VP64_ASSERT(n != 0);

  const size_t max_partition =
      partition_length[ARRAYSIZE(partition_length) - 1];

  if (max_partition +
        MAX_UNPACK_OVERRUN_NUM > n) {
    for (size_t i = 0; i < n; i++)
      dst[i] = DecodeUint64(src + 8 * i);

    return n * 8;
  }

  /*
   * A functional pointer below maps a
   * given control byte to the corresponding
   * unpacker function. Higher 4-bits in the
   * control byte means a index in the
   * functional pointer.
   */
  typedef int (*vpack64_t)(
      const char *restrict src,
      const char *restrict slimit,
      uint64_t *restrict dst,
      const uint64_t *restrict dlimit, int n);

  static vpack64_t vpacker64[16] = {
    Unpack0, Unpack1, Unpack2, Unpack3,
    Unpack4, Unpack5, Unpack6, Unpack7,
    Unpack8, Unpack9, Unpack10, Unpack11,
    Unpack12, Unpack16, Unpack32, Unpack64
  };

  /* Ready for decompression */
  uint32_t block_size = DecodeUint32(src);
  uint32_t offset = DecodeUint32(src + 4);

  const char *slimit = src + block_size;
  const uint64_t *dlimit = dst + n;

  const char *ctrl = src + 8;
  const char *data = src + offset;

  /* Do decompression */
  int nloop = data - ctrl;

  for (int i = 0; i < nloop; i++) {
    size_t k = partition_length[(*ctrl >> 4) & 0x0f];
    int b = *ctrl & 0x0f;

    /* Do unpacking */
    int nread = vpacker64[b](
        data, slimit, dst, dlimit, k);
    if (nread < 0)
      return 0;

    data += nread;
    dst += k;
    ctrl++;
  }

  /* Copy left bytes to a output */
  for (size_t i = 0;
        i < MAX_UNPACK_OVERRUN_NUM; i++)
    dst[i] = DecodeUint64(data + 8 * i);

  return block_size;
}

} /* namespace: backend */

using namespace vpacker64::backend;

/*-------------------------------------------------
 * The function provides the maximumx size that
 * Compress() may output. It is useful to know
 * the size in advance because of memory
 * allocation for *dst in Compress().
 *
 *  n      : # of input integers
 *  return : maximum size Compess() may outputs
 *-------------------------------------------------
 */
inline size_t CompressBound(size_t n) {
  size_t nblock =
      VP64_DIV_ROUNDUP(n, block_num);

  return 8 + 8 * nblock + 9 * n;
}


/*-------------------------------------------------
 * A simple interface for compression
 *
 *  src    : input buffer
 *  dst    : output buffer
 *  n      : # of input integers
 *  return : # of written bytes in Compress()
 *-------------------------------------------------
 */
inline size_t Compress(const uint64_t *src,
                       char *dst,
                       size_t n) {
  if (src == NULL || dst == NULL)
    return 0;

  size_t nblock = n / block_num;
  size_t rblock = n % block_num;

  char *dlimit = dst + CompressBound(n);

  /* Write down a magic number */
  SetUint64(dst, VP64_MAGICNUM);
  dst += 8;
  size_t wsize = 8;

  for (size_t i = 0; i < nblock; i++) {
    uint32_t nwrite =
        CompressBlock(src, block_num, dst, dlimit);
    if (nwrite == 0)
      return 0;

    /* Move to a next block */
    src += block_num;
    dst += nwrite;
    wsize += nwrite;
  }

  /* Compress left elements in src */
  if (rblock) {
    uint32_t nwrite =
        CompressBlock(src, rblock, dst, dlimit);
    if (nwrite == 0)
      return 0;

    wsize += nwrite;
  }

  return wsize;
}


/*-------------------------------------------------
 * A simple interface for decompression
 *
 *  src    : input buffer
 *  dst    : output buffer
 *  n      : # of input bytes
 *  return : # of read bytes in Uncompress()
 *-------------------------------------------------
 */
inline size_t Uncompress(const char *src,
                         uint64_t *dst,
                         size_t n) {
  if (src == NULL || dst == NULL)
    return 0;

  /* Check if a magic number is correct */
  if (VP64_MAGICNUM != DecodeUint64(src))
    return 0;

  src += 8;
  size_t rsize = 8;

  size_t nblock = n / block_num;
  size_t rblock = n % block_num;

  for (size_t i = 0; i < nblock; i++) {
    uint32_t nread =
        UncompressBlock(src, dst, block_num);

    /* Check if it works correctly */
    if (nread == 0)
      return 0;

    /* Move to a next block */
    src += nread;
    dst += block_num;
    rsize += nread;
  }

  if (rblock) {
    uint32_t nread =
        UncompressBlock(src, dst, rblock);

    /* Check if it works correctly */
    if (nread == 0)
      return 0;

    rsize += nread;
  }

  return rsize;
}

} /* namespace: vpacker64 */

#endif /* __INCLUDE_VPACKER64_HPP__ */
