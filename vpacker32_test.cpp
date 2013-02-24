/*-----------------------------------------------------------------------------
 *  vpacker32_test.cpp - A test set for vpacker32.hpp
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2013 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

#include <vpacker32.hpp>
#include <vpacker_test.hpp>

/* Not display some warnings in gcc */
#if defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wall"
# pragma GCC diagnostic ignored "-Wextra"
# pragma GCC diagnostic ignored "-Wsign-compare"
#endif

#include <gtest/gtest.h>

#if defined(__GNUC__)
# pragma GCC diagnostic pop
#endif

using namespace vpacker32;
using namespace vpacker32::backend;

class Vpacker32P :
    public testing::TestWithParam<size_t> {
 public:
  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_P(Vpacker32P, Compress) {
  TestDataMgr<uint32_t> tmgr;
  std::vector<uint32_t> tv;

  size_t    num = GetParam();
  size_t    dbound = CompressBound(num);
  char     *dst = new char[dbound];
  uint32_t *buf = new uint32_t[num];

  uint32_t  range[] = {
    1ULL << 1, 1ULL << 2, 1ULL << 3,
    1ULL << 4, 1ULL << 5, 1ULL << 6,
    1ULL << 7, 1ULL << 8, 1ULL << 9,
    1ULL << 10, 1ULL << 11, 1ULL << 12,
    1ULL << 16, 1ULL << 24
  };

  for (size_t i = 0;
        i < ARRAYSIZE(range); i++) {
    const uint32_t *dv =
        tmgr.generate(&tv, num, range[i]);

    uint32_t wsz = Compress(dv, dst, num);
    ASSERT_TRUE(wsz <= dbound);

    uint32_t rsz = Uncompress(dst, buf, num);

    EXPECT_EQ(rsz, wsz);
    for (size_t i = 0; i < num; i++)
      EXPECT_EQ(dv[i], buf[i]);

    /*
     * Re-write the magic number, and check
     * if a crruption occurs.
     */
    SetUint64(dst, 0x0fbc32ad23902394);
    EXPECT_EQ(0, Uncompress(dst, buf, num));
  }

  delete[] dst;
  delete[] buf;
}

TEST_P(Vpacker32P, CompressBlock) {
  TestDataMgr<uint32_t> tmgr;
  std::vector<uint32_t> tv;

  size_t    num = GetParam();
  size_t    dbound = CompressBound(num);
  char     *dst = new char[dbound];
  char     *dlimit = dst + dbound;
  uint32_t *buf = new uint32_t[num];

  uint32_t  range[] = {
    1ULL << 1, 1ULL << 2, 1ULL << 3,
    1ULL << 4, 1ULL << 5, 1ULL << 6,
    1ULL << 7, 1ULL << 8, 1ULL << 9,
    1ULL << 10, 1ULL << 11, 1ULL << 12,
    1ULL << 16, 1ULL << 24
  };

  for (size_t i = 0;
        i < ARRAYSIZE(range); i++) {
    const uint32_t *dv =
        tmgr.generate(&tv, num, range[i]);

    uint32_t wsz =
        CompressBlock(dv, num, dst, dlimit);
    ASSERT_TRUE(wsz <= dbound);

    uint32_t rsz =
        UncompressBlock(dst, buf, num);

    EXPECT_EQ(rsz, wsz);
    for (size_t i = 0; i < num; i++)
      EXPECT_EQ(dv[i], buf[i]);
  }

  delete[] dst;
  delete[] buf;
}

/* Generate a sequence of tests */
INSTANTIATE_TEST_CASE_P(
    Vpacker32PSmall, Vpacker32P,
    ::testing::Range<size_t>(1, 256));

INSTANTIATE_TEST_CASE_P(
    Vpacker32PSparse, Vpacker32P,
    ::testing::Values(1024, 2048, 4096,
                      8192, 16384, 32768, 65536, 131072));

TEST(Vpacker32, SetUint32) {
  char  buf[4];

  SetUint32(buf, 2169682782);
  EXPECT_EQ('\x81', buf[0]);
  EXPECT_EQ('\x52', buf[1]);
  EXPECT_EQ('\xbb', buf[2]);
  EXPECT_EQ('\x5e', buf[3]);

  EXPECT_EQ(33106, DecodeUint16(buf));
  EXPECT_EQ(47966, DecodeUint16(buf + 2));
  EXPECT_EQ(2169682782, DecodeUint32(buf));

  SetUint32(buf, 973589125);
  EXPECT_EQ('\x3a', buf[0]);
  EXPECT_EQ('\x07', buf[1]);
  EXPECT_EQ('\xca', buf[2]);
  EXPECT_EQ('\x85', buf[3]);

  EXPECT_EQ(14855, DecodeUint16(buf));
  EXPECT_EQ(51845, DecodeUint16(buf + 2));
  EXPECT_EQ(973589125, DecodeUint32(buf));
}

TEST(Vpacker64, SetUint64) {
  char  buf[8];

  SetUint64(buf, 90285902385930821ULL);
  EXPECT_EQ('\x01', buf[0]);
  EXPECT_EQ('\x40', buf[1]);
  EXPECT_EQ('\xc2', buf[2]);
  EXPECT_EQ('\x8c', buf[3]);
  EXPECT_EQ('\xc3', buf[4]);
  EXPECT_EQ('\xf0', buf[5]);
  EXPECT_EQ('\x62', buf[6]);
  EXPECT_EQ('\x45', buf[7]);

  EXPECT_EQ(320, DecodeUint16(buf));
  EXPECT_EQ(49804, DecodeUint16(buf + 2));
  EXPECT_EQ(50160, DecodeUint16(buf + 4));
  EXPECT_EQ(25157, DecodeUint16(buf + 6));
  EXPECT_EQ(21021324, DecodeUint32(buf));
  EXPECT_EQ(3287310917, DecodeUint32(buf + 4));
  EXPECT_EQ(90285902385930821ULL, DecodeUint64(buf));

  SetUint64(buf, 2546335145698555275);
  EXPECT_EQ('\x23', buf[0]);
  EXPECT_EQ('\x56', buf[1]);
  EXPECT_EQ('\x66', buf[2]);
  EXPECT_EQ('\x52', buf[3]);
  EXPECT_EQ('\xee', buf[4]);
  EXPECT_EQ('\x74', buf[5]);
  EXPECT_EQ('\x75', buf[6]);
  EXPECT_EQ('\x8b', buf[7]);

  EXPECT_EQ(9046, DecodeUint16(buf));
  EXPECT_EQ(26194, DecodeUint16(buf + 2));
  EXPECT_EQ(61044, DecodeUint16(buf + 4));
  EXPECT_EQ(30091, DecodeUint16(buf + 6));
  EXPECT_EQ(592864850, DecodeUint32(buf));
  EXPECT_EQ(4000609675, DecodeUint32(buf + 4));
  EXPECT_EQ(2546335145698555275ULL, DecodeUint64(buf));
}

TEST(Vpacker32, Unpack0) {
  const char     *src = "0x00"; /* Not used in Unpack0 */
  uint32_t        dst[32];
  const char     *slimit = src;
  const uint32_t *dlimit = dst + 32;

  memset(dst, 0xff, 256);

  EXPECT_EQ(0, Unpack0(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(0, dst[0]);

  EXPECT_EQ(0, Unpack0(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(0, dst[0]);
  EXPECT_EQ(0, dst[1]);

  EXPECT_EQ(0, Unpack0(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(0, dst[0]);
  EXPECT_EQ(0, dst[1]);
  EXPECT_EQ(0, dst[2]);

  EXPECT_EQ(0, Unpack0(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(0, dst[0]);
  EXPECT_EQ(0, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(0, dst[3]);

  EXPECT_EQ(0, Unpack0(
          src, slimit, dst, dlimit, 9));
  for (int i = 0; i < 9; i++)
    EXPECT_EQ(0, dst[i]);

  EXPECT_EQ(0, Unpack0(
          src, slimit, dst, dlimit, 17));
  for (int i = 0; i < 17; i++)
    EXPECT_EQ(0, dst[i]);

  EXPECT_EQ(0, Unpack0(
          src, slimit, dst, dlimit, 25));
  for (int i = 0; i < 25; i++)
    EXPECT_EQ(0, dst[i]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack0(
          src, slimit, dst, dlimit, 33));
  EXPECT_EQ(-1, Unpack0(
          src, slimit, dst, dst, 1));
}

TEST(Vpacker32, Unpack1) {
  const char     *src = "\xcc\xff";
  uint32_t        dst[16];
  const char     *slimit = src + 2;
  const uint32_t *dlimit = dst + 16;

  memset(dst, 0xff, 128);

  EXPECT_EQ(1, Unpack1(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(1, dst[0]);

  EXPECT_EQ(1, Unpack1(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(1, dst[1]);

  EXPECT_EQ(1, Unpack1(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(1, dst[1]);
  EXPECT_EQ(0, dst[2]);

  EXPECT_EQ(1, Unpack1(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(1, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(0, dst[3]);

  EXPECT_EQ(1, Unpack1(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(1, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(0, dst[3]);
  EXPECT_EQ(1, dst[4]);

  EXPECT_EQ(2, Unpack1(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(1, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(0, dst[3]);
  EXPECT_EQ(1, dst[4]);
  EXPECT_EQ(1, dst[5]);
  EXPECT_EQ(0, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(1, dst[8]);

  EXPECT_EQ(2, Unpack1(
          src, slimit, dst, dlimit, 16));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(1, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(0, dst[3]);
  EXPECT_EQ(1, dst[4]);
  EXPECT_EQ(1, dst[5]);
  EXPECT_EQ(0, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(1, dst[8]);
  EXPECT_EQ(1, dst[9]);
  EXPECT_EQ(1, dst[10]);
  EXPECT_EQ(1, dst[11]);
  EXPECT_EQ(1, dst[12]);
  EXPECT_EQ(1, dst[13]);
  EXPECT_EQ(1, dst[14]);
  EXPECT_EQ(1, dst[15]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack1(
          src, src, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack1(
          src, src + 1, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack1(
          src, slimit, dst, dlimit, 17));
  EXPECT_EQ(-1, Unpack1(
          src, slimit, dst, dst, 1));
}

TEST(Vpacker32, Unpack2) {
  const char     *src = "\xc2\x4b";
  uint32_t        dst[8];
  const char     *slimit = src + 2;
  const uint32_t *dlimit = dst + 8;

  memset(dst, 0xff, 64);

  EXPECT_EQ(1, Unpack2(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(3, dst[0]);

  EXPECT_EQ(1, Unpack2(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(3, dst[0]);
  EXPECT_EQ(0, dst[1]);

  EXPECT_EQ(1, Unpack2(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(3, dst[0]);
  EXPECT_EQ(0, dst[1]);
  EXPECT_EQ(0, dst[2]);

  EXPECT_EQ(1, Unpack2(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(3, dst[0]);
  EXPECT_EQ(0, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(2, dst[3]);

  EXPECT_EQ(2, Unpack2(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(3, dst[0]);
  EXPECT_EQ(0, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(2, dst[3]);
  EXPECT_EQ(1, dst[4]);

  EXPECT_EQ(2, Unpack2(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(3, dst[0]);
  EXPECT_EQ(0, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(2, dst[3]);
  EXPECT_EQ(1, dst[4]);
  EXPECT_EQ(0, dst[5]);

  EXPECT_EQ(2, Unpack2(
          src, slimit, dst, dlimit, 7));
  EXPECT_EQ(3, dst[0]);
  EXPECT_EQ(0, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(2, dst[3]);
  EXPECT_EQ(1, dst[4]);
  EXPECT_EQ(0, dst[5]);
  EXPECT_EQ(2, dst[6]);

  EXPECT_EQ(2, Unpack2(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(3, dst[0]);
  EXPECT_EQ(0, dst[1]);
  EXPECT_EQ(0, dst[2]);
  EXPECT_EQ(2, dst[3]);
  EXPECT_EQ(1, dst[4]);
  EXPECT_EQ(0, dst[5]);
  EXPECT_EQ(2, dst[6]);
  EXPECT_EQ(3, dst[7]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack2(
          src, src, dst, dlimit, 8));
  EXPECT_EQ(-1, Unpack2(
          src, src + 1, dst, dlimit, 8));
  EXPECT_EQ(-1, Unpack2(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(-1, Unpack2(
          src, slimit, dst, dst, 8));
}

TEST(Vpacker32, Unpack3) {
  const char     *src = "\x3a\x94\xff\x0a\xd3\x22";
  uint32_t        dst[16];
  const char     *slimit = src + 6;
  const uint32_t *dlimit = dst + 16;

  memset(dst, 0xff, 64);

  EXPECT_EQ(1, Unpack3(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(1, dst[0]);

  EXPECT_EQ(1, Unpack3(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);

  EXPECT_EQ(2, Unpack3(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);

  EXPECT_EQ(2, Unpack3(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);
  EXPECT_EQ(1, dst[3]);

  EXPECT_EQ(2, Unpack3(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);
  EXPECT_EQ(1, dst[3]);
  EXPECT_EQ(2, dst[4]);

  EXPECT_EQ(3, Unpack3(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);
  EXPECT_EQ(1, dst[3]);
  EXPECT_EQ(2, dst[4]);
  EXPECT_EQ(3, dst[5]);

  EXPECT_EQ(3, Unpack3(
          src, slimit, dst, dlimit, 7));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);
  EXPECT_EQ(1, dst[3]);
  EXPECT_EQ(2, dst[4]);
  EXPECT_EQ(3, dst[5]);
  EXPECT_EQ(7, dst[6]);

  EXPECT_EQ(3, Unpack3(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);
  EXPECT_EQ(1, dst[3]);
  EXPECT_EQ(2, dst[4]);
  EXPECT_EQ(3, dst[5]);
  EXPECT_EQ(7, dst[6]);
  EXPECT_EQ(7, dst[7]);

  EXPECT_EQ(4, Unpack3(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);
  EXPECT_EQ(1, dst[3]);
  EXPECT_EQ(2, dst[4]);
  EXPECT_EQ(3, dst[5]);
  EXPECT_EQ(7, dst[6]);
  EXPECT_EQ(7, dst[7]);
  EXPECT_EQ(0, dst[8]);

  EXPECT_EQ(4, Unpack3(
          src, slimit, dst, dlimit, 10));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);
  EXPECT_EQ(1, dst[3]);
  EXPECT_EQ(2, dst[4]);
  EXPECT_EQ(3, dst[5]);
  EXPECT_EQ(7, dst[6]);
  EXPECT_EQ(7, dst[7]);
  EXPECT_EQ(0, dst[8]);
  EXPECT_EQ(2, dst[9]);

  EXPECT_EQ(6, Unpack3(
          src, slimit, dst, dlimit, 16));
  EXPECT_EQ(1, dst[0]);
  EXPECT_EQ(6, dst[1]);
  EXPECT_EQ(5, dst[2]);
  EXPECT_EQ(1, dst[3]);
  EXPECT_EQ(2, dst[4]);
  EXPECT_EQ(3, dst[5]);
  EXPECT_EQ(7, dst[6]);
  EXPECT_EQ(7, dst[7]);
  EXPECT_EQ(0, dst[8]);
  EXPECT_EQ(2, dst[9]);
  EXPECT_EQ(5, dst[10]);
  EXPECT_EQ(5, dst[11]);
  EXPECT_EQ(1, dst[12]);
  EXPECT_EQ(4, dst[13]);
  EXPECT_EQ(4, dst[14]);
  EXPECT_EQ(2, dst[15]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack3(
          src, src, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack3(
          src, src + 1, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack3(
          src, slimit, dst, dlimit, 17));
  EXPECT_EQ(-1, Unpack3(
          src, slimit, dst, dst, 16));
}

TEST(Vpacker32, Unpack4) {
  const char     *src = "\x9a\x28";
  uint32_t        dst[4];
  const char     *slimit = src + 2;
  const uint32_t *dlimit = dst + 4;

  memset(dst, 0xff, 32);

  EXPECT_EQ(1, Unpack4(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(9, dst[0]);

  EXPECT_EQ(1, Unpack4(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(9, dst[0]);
  EXPECT_EQ(10, dst[1]);

  EXPECT_EQ(2, Unpack4(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(9, dst[0]);
  EXPECT_EQ(10, dst[1]);
  EXPECT_EQ(2, dst[2]);

  EXPECT_EQ(2, Unpack4(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(9, dst[0]);
  EXPECT_EQ(10, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(8, dst[3]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack4(
          src, src, dst, dlimit, 4));
  EXPECT_EQ(-1, Unpack4(
          src, src + 1, dst, dlimit, 4));
  EXPECT_EQ(-1, Unpack4(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(-1, Unpack4(
          src, slimit, dst, dst, 4));
}

TEST(Vpacker32, Unpack5) {
  const char      src[10] = {
    0xfd, 0x11, 0x93, 0x23, 0xc0, 0x02, 0x83, 0x99,
    0xbb, 0xcd
  };
  uint32_t        dst[16];
  const char     *slimit = src + 10;
  const uint32_t *dlimit = dst + 16;

  memset(dst, 0xff, 128);

  EXPECT_EQ(1, Unpack5(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(31, dst[0]);

  EXPECT_EQ(2, Unpack5(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);

  EXPECT_EQ(2, Unpack5(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);

  EXPECT_EQ(3, Unpack5(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);
  EXPECT_EQ(25, dst[3]);

  EXPECT_EQ(4, Unpack5(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);
  EXPECT_EQ(25, dst[3]);
  EXPECT_EQ(6, dst[4]);

  EXPECT_EQ(4, Unpack5(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);
  EXPECT_EQ(25, dst[3]);
  EXPECT_EQ(6, dst[4]);
  EXPECT_EQ(8, dst[5]);

  EXPECT_EQ(5, Unpack5(
          src, slimit, dst, dlimit, 7));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);
  EXPECT_EQ(25, dst[3]);
  EXPECT_EQ(6, dst[4]);
  EXPECT_EQ(8, dst[5]);
  EXPECT_EQ(30, dst[6]);

  EXPECT_EQ(5, Unpack5(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);
  EXPECT_EQ(25, dst[3]);
  EXPECT_EQ(6, dst[4]);
  EXPECT_EQ(8, dst[5]);
  EXPECT_EQ(30, dst[6]);
  EXPECT_EQ(0, dst[7]);

  EXPECT_EQ(6, Unpack5(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);
  EXPECT_EQ(25, dst[3]);
  EXPECT_EQ(6, dst[4]);
  EXPECT_EQ(8, dst[5]);
  EXPECT_EQ(30, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(0, dst[8]);

  EXPECT_EQ(7, Unpack5(
          src, slimit, dst, dlimit, 10));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);
  EXPECT_EQ(25, dst[3]);
  EXPECT_EQ(6, dst[4]);
  EXPECT_EQ(8, dst[5]);
  EXPECT_EQ(30, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(0, dst[8]);
  EXPECT_EQ(10, dst[9]);

  EXPECT_EQ(10, Unpack5(
          src, slimit, dst, dlimit, 16));
  EXPECT_EQ(31, dst[0]);
  EXPECT_EQ(20, dst[1]);
  EXPECT_EQ(8, dst[2]);
  EXPECT_EQ(25, dst[3]);
  EXPECT_EQ(6, dst[4]);
  EXPECT_EQ(8, dst[5]);
  EXPECT_EQ(30, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(0, dst[8]);
  EXPECT_EQ(10, dst[9]);
  EXPECT_EQ(1, dst[10]);
  EXPECT_EQ(25, dst[11]);
  EXPECT_EQ(19, dst[12]);
  EXPECT_EQ(14, dst[13]);
  EXPECT_EQ(30, dst[14]);
  EXPECT_EQ(13, dst[15]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack5(
          src, src, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack5(
          src, src + 1, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack5(
          src, slimit, dst, dlimit, 17));
  EXPECT_EQ(-1, Unpack5(
          src, slimit, dst, dst, 16));
}

TEST(Vpacker32, Unpack6) {
  const char     *src = "\x22\xaf\x9c\x01\x28\xbb";
  uint32_t        dst[8];
  const char     *slimit = src + 6;
  const uint32_t *dlimit = dst + 8;

  memset(dst, 0xff, 64);

  EXPECT_EQ(1, Unpack6(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(8, dst[0]);

  EXPECT_EQ(2, Unpack6(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(8, dst[0]);
  EXPECT_EQ(42, dst[1]);

  EXPECT_EQ(3, Unpack6(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(8, dst[0]);
  EXPECT_EQ(42, dst[1]);
  EXPECT_EQ(62, dst[2]);

  EXPECT_EQ(3, Unpack6(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(8, dst[0]);
  EXPECT_EQ(42, dst[1]);
  EXPECT_EQ(62, dst[2]);
  EXPECT_EQ(28, dst[3]);

  EXPECT_EQ(4, Unpack6(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(8, dst[0]);
  EXPECT_EQ(42, dst[1]);
  EXPECT_EQ(62, dst[2]);
  EXPECT_EQ(28, dst[3]);
  EXPECT_EQ(0, dst[4]);

  EXPECT_EQ(5, Unpack6(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(8, dst[0]);
  EXPECT_EQ(42, dst[1]);
  EXPECT_EQ(62, dst[2]);
  EXPECT_EQ(28, dst[3]);
  EXPECT_EQ(0, dst[4]);
  EXPECT_EQ(18, dst[5]);

  EXPECT_EQ(6, Unpack6(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(8, dst[0]);
  EXPECT_EQ(42, dst[1]);
  EXPECT_EQ(62, dst[2]);
  EXPECT_EQ(28, dst[3]);
  EXPECT_EQ(0, dst[4]);
  EXPECT_EQ(18, dst[5]);
  EXPECT_EQ(34, dst[6]);
  EXPECT_EQ(59, dst[7]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack6(
          src, src, dst, dlimit, 8));
  EXPECT_EQ(-1, Unpack6(
          src, src + 1, dst, dlimit, 8));
  EXPECT_EQ(-1, Unpack6(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(-1, Unpack6(
          src, slimit, dst, dst, 8));
}

TEST(Vpacker32, Unpack7) {
  const char      src[14] = {
    0xdd, 0x00, 0x10, 0x93, 0xcd, 0xc1, 0x29, 0x81,
    0xa0, 0x03, 0x43, 0x10, 0x88, 0x0a
  };
  uint32_t        dst[16];
  const char     *slimit = src + 14;
  const uint32_t *dlimit = dst + 16;

  memset(dst, 0xff, 128);

  EXPECT_EQ(1, Unpack7(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(110, dst[0]);

  EXPECT_EQ(2, Unpack7(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);

  EXPECT_EQ(3, Unpack7(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);

  EXPECT_EQ(4, Unpack7(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(9, dst[3]);

  EXPECT_EQ(5, Unpack7(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(9, dst[3]);
  EXPECT_EQ(30, dst[4]);

  EXPECT_EQ(6, Unpack7(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(9, dst[3]);
  EXPECT_EQ(30, dst[4]);
  EXPECT_EQ(55, dst[5]);

  EXPECT_EQ(7, Unpack7(
          src, slimit, dst, dlimit, 7));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(9, dst[3]);
  EXPECT_EQ(30, dst[4]);
  EXPECT_EQ(55, dst[5]);
  EXPECT_EQ(2, dst[6]);

  EXPECT_EQ(7, Unpack7(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(9, dst[3]);
  EXPECT_EQ(30, dst[4]);
  EXPECT_EQ(55, dst[5]);
  EXPECT_EQ(2, dst[6]);
  EXPECT_EQ(41, dst[7]);

  EXPECT_EQ(8, Unpack7(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(9, dst[3]);
  EXPECT_EQ(30, dst[4]);
  EXPECT_EQ(55, dst[5]);
  EXPECT_EQ(2, dst[6]);
  EXPECT_EQ(41, dst[7]);
  EXPECT_EQ(64, dst[8]);

  EXPECT_EQ(9, Unpack7(
          src, slimit, dst, dlimit, 10));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(9, dst[3]);
  EXPECT_EQ(30, dst[4]);
  EXPECT_EQ(55, dst[5]);
  EXPECT_EQ(2, dst[6]);
  EXPECT_EQ(41, dst[7]);
  EXPECT_EQ(64, dst[8]);
  EXPECT_EQ(104, dst[9]);

  EXPECT_EQ(14, Unpack7(
          src, slimit, dst, dlimit, 16));
  EXPECT_EQ(110, dst[0]);
  EXPECT_EQ(64, dst[1]);
  EXPECT_EQ(2, dst[2]);
  EXPECT_EQ(9, dst[3]);
  EXPECT_EQ(30, dst[4]);
  EXPECT_EQ(55, dst[5]);
  EXPECT_EQ(2, dst[6]);
  EXPECT_EQ(41, dst[7]);
  EXPECT_EQ(64, dst[8]);
  EXPECT_EQ(104, dst[9]);
  EXPECT_EQ(0, dst[10]);
  EXPECT_EQ(52, dst[11]);
  EXPECT_EQ(24, dst[12]);
  EXPECT_EQ(66, dst[13]);
  EXPECT_EQ(16, dst[14]);
  EXPECT_EQ(10, dst[15]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack7(
          src, src, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack7(
          src, src + 1, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack7(
          src, slimit, dst, dlimit, 17));
  EXPECT_EQ(-1, Unpack7(
          src, slimit, dst, dst, 16));
}

TEST(Vpacker32, Unpack8) {
  const char     *src = "\x9a\x28";
  uint32_t        dst[2];
  const char     *slimit = src + 2;
  const uint32_t *dlimit = dst + 2;

  memset(dst, 0xff, 16);

  EXPECT_EQ(1, Unpack8(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(154, dst[0]);

  EXPECT_EQ(2, Unpack8(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(154, dst[0]);
  EXPECT_EQ(40, dst[1]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack8(
          src, src, dst, dlimit, 2));
  EXPECT_EQ(-1, Unpack8(
          src, src + 1, dst, dlimit, 2));
  EXPECT_EQ(-1, Unpack8(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(-1, Unpack8(
          src, slimit, dst, dst, 2));
}

TEST(Vpacker32, Unpack9) {
  const char      src[36] = {
    0xab, 0x82, 0x33, 0x24, 0x32, 0xac, 0x8d, 0x8a,
    0x00, 0xd8, 0xf0, 0xf8, 0x22, 0x67, 0x26, 0xd7,
    0x83, 0xaa, 0x02, 0xc8, 0x2a, 0xca, 0x28, 0x82,
    0x64, 0xca, 0x83, 0x1a, 0x00, 0x00, 0x1a, 0xf1,
    0x23, 0xab, 0xff, 0x32
  };
  uint32_t        dst[32];
  const char     *slimit = src + 36;
  const uint32_t *dlimit = dst + 32;

  memset(dst, 0xff, 256);

  EXPECT_EQ(2, Unpack9(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(343, dst[0]);

  EXPECT_EQ(3, Unpack9(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);

  EXPECT_EQ(4, Unpack9(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);

  EXPECT_EQ(5, Unpack9(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);

  EXPECT_EQ(6, Unpack9(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);

  EXPECT_EQ(7, Unpack9(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);

  EXPECT_EQ(8, Unpack9(
          src, slimit, dst, dlimit, 7));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);

  EXPECT_EQ(9, Unpack9(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);

  EXPECT_EQ(11, Unpack9(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);

  EXPECT_EQ(12, Unpack9(
          src, slimit, dst, dlimit, 10));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);

  EXPECT_EQ(13, Unpack9(
          src, slimit, dst, dlimit, 11));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);

  EXPECT_EQ(14, Unpack9(
          src, slimit, dst, dlimit, 12));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);
  EXPECT_EQ(38, dst[11]);

  EXPECT_EQ(15, Unpack9(
          src, slimit, dst, dlimit, 13));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);
  EXPECT_EQ(38, dst[11]);
  EXPECT_EQ(228, dst[12]);

  EXPECT_EQ(16, Unpack9(
          src, slimit, dst, dlimit, 14));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);
  EXPECT_EQ(38, dst[11]);
  EXPECT_EQ(228, dst[12]);
  EXPECT_EQ(437, dst[13]);

  EXPECT_EQ(17, Unpack9(
          src, slimit, dst, dlimit, 15));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);
  EXPECT_EQ(38, dst[11]);
  EXPECT_EQ(228, dst[12]);
  EXPECT_EQ(437, dst[13]);
  EXPECT_EQ(449, dst[14]);

  EXPECT_EQ(18, Unpack9(
          src, slimit, dst, dlimit, 16));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);
  EXPECT_EQ(38, dst[11]);
  EXPECT_EQ(228, dst[12]);
  EXPECT_EQ(437, dst[13]);
  EXPECT_EQ(449, dst[14]);
  EXPECT_EQ(426, dst[15]);

  EXPECT_EQ(20, Unpack9(
          src, slimit, dst, dlimit, 17));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);
  EXPECT_EQ(38, dst[11]);
  EXPECT_EQ(228, dst[12]);
  EXPECT_EQ(437, dst[13]);
  EXPECT_EQ(449, dst[14]);
  EXPECT_EQ(426, dst[15]);
  EXPECT_EQ(5, dst[16]);

  EXPECT_EQ(21, Unpack9(
          src, slimit, dst, dlimit, 18));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);
  EXPECT_EQ(38, dst[11]);
  EXPECT_EQ(228, dst[12]);
  EXPECT_EQ(437, dst[13]);
  EXPECT_EQ(449, dst[14]);
  EXPECT_EQ(426, dst[15]);
  EXPECT_EQ(5, dst[16]);
  EXPECT_EQ(288, dst[17]);

  EXPECT_EQ(36, Unpack9(
          src, slimit, dst, dlimit, 32));
  EXPECT_EQ(343, dst[0]);
  EXPECT_EQ(8, dst[1]);
  EXPECT_EQ(409, dst[2]);
  EXPECT_EQ(67, dst[3]);
  EXPECT_EQ(85, dst[4]);
  EXPECT_EQ(291, dst[5]);
  EXPECT_EQ(197, dst[6]);
  EXPECT_EQ(0, dst[7]);
  EXPECT_EQ(433, dst[8]);
  EXPECT_EQ(451, dst[9]);
  EXPECT_EQ(449, dst[10]);
  EXPECT_EQ(38, dst[11]);
  EXPECT_EQ(228, dst[12]);
  EXPECT_EQ(437, dst[13]);
  EXPECT_EQ(449, dst[14]);
  EXPECT_EQ(426, dst[15]);
  EXPECT_EQ(5, dst[16]);
  EXPECT_EQ(288, dst[17]);
  EXPECT_EQ(342, dst[18]);
  EXPECT_EQ(162, dst[19]);
  EXPECT_EQ(272, dst[20]);
  EXPECT_EQ(153, dst[21]);
  EXPECT_EQ(101, dst[22]);
  EXPECT_EQ(131, dst[23]);
  EXPECT_EQ(52, dst[24]);
  EXPECT_EQ(0, dst[25]);
  EXPECT_EQ(0, dst[26]);
  EXPECT_EQ(431, dst[27]);
  EXPECT_EQ(36, dst[28]);
  EXPECT_EQ(234, dst[29]);
  EXPECT_EQ(511, dst[30]);
  EXPECT_EQ(306, dst[31]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack9(
          src, src, dst, dlimit, 32));
  EXPECT_EQ(-1, Unpack9(
          src, src + 1, dst, dlimit, 32));
  EXPECT_EQ(-1, Unpack9(
          src, slimit, dst, dlimit, 33));
  EXPECT_EQ(-1, Unpack9(
          src, slimit, dst, dst, 32));
}

TEST(Vpacker32, Unpack10) {
  const char      src[] = {
    0x3f, 0x20, 0x21, 0xab, 0x93, 0xd3, 0xb2, 0x32,
    0x8b, 0x72, 0xc8, 0x3c, 0xf0, 0x00, 0x30, 0xcc,
    0x23, 0x93, 0x0a, 0xd0
  };
  uint32_t        dst[16];
  const char     *slimit = src + 20;
  const uint32_t *dlimit = dst + 16;

  memset(dst, 0xff, 128);

  EXPECT_EQ(2, Unpack10(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(252, dst[0]);

  EXPECT_EQ(3, Unpack10(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);

  EXPECT_EQ(4, Unpack10(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);

  EXPECT_EQ(5, Unpack10(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);
  EXPECT_EQ(915, dst[3]);

  EXPECT_EQ(7, Unpack10(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);
  EXPECT_EQ(915, dst[3]);
  EXPECT_EQ(846, dst[4]);

  EXPECT_EQ(8, Unpack10(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);
  EXPECT_EQ(915, dst[3]);
  EXPECT_EQ(846, dst[4]);
  EXPECT_EQ(803, dst[5]);

  EXPECT_EQ(9, Unpack10(
          src, slimit, dst, dlimit, 7));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);
  EXPECT_EQ(915, dst[3]);
  EXPECT_EQ(846, dst[4]);
  EXPECT_EQ(803, dst[5]);
  EXPECT_EQ(162, dst[6]);

  EXPECT_EQ(10, Unpack10(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);
  EXPECT_EQ(915, dst[3]);
  EXPECT_EQ(846, dst[4]);
  EXPECT_EQ(803, dst[5]);
  EXPECT_EQ(162, dst[6]);
  EXPECT_EQ(882, dst[7]);

  EXPECT_EQ(12, Unpack10(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);
  EXPECT_EQ(915, dst[3]);
  EXPECT_EQ(846, dst[4]);
  EXPECT_EQ(803, dst[5]);
  EXPECT_EQ(162, dst[6]);
  EXPECT_EQ(882, dst[7]);
  EXPECT_EQ(800, dst[8]);

  EXPECT_EQ(13, Unpack10(
          src, slimit, dst, dlimit, 10));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);
  EXPECT_EQ(915, dst[3]);
  EXPECT_EQ(846, dst[4]);
  EXPECT_EQ(803, dst[5]);
  EXPECT_EQ(162, dst[6]);
  EXPECT_EQ(882, dst[7]);
  EXPECT_EQ(800, dst[8]);
  EXPECT_EQ(975, dst[9]);

  EXPECT_EQ(20, Unpack10(
          src, slimit, dst, dlimit, 16));
  EXPECT_EQ(252, dst[0]);
  EXPECT_EQ(514, dst[1]);
  EXPECT_EQ(106, dst[2]);
  EXPECT_EQ(915, dst[3]);
  EXPECT_EQ(846, dst[4]);
  EXPECT_EQ(803, dst[5]);
  EXPECT_EQ(162, dst[6]);
  EXPECT_EQ(882, dst[7]);
  EXPECT_EQ(800, dst[8]);
  EXPECT_EQ(975, dst[9]);
  EXPECT_EQ(0, dst[10]);
  EXPECT_EQ(48, dst[11]);
  EXPECT_EQ(816, dst[12]);
  EXPECT_EQ(569, dst[13]);
  EXPECT_EQ(194, dst[14]);
  EXPECT_EQ(720, dst[15]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack10(
          src, src, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack10(
          src, src + 1, dst, dlimit, 16));
  EXPECT_EQ(-1, Unpack10(
          src, slimit, dst, dlimit, 17));
  EXPECT_EQ(-1, Unpack10(
          src, slimit, dst, dst, 16));
}

TEST(Vpacker32, Unpack11) {
  const char      src[44] = {
    0x19, 0x93, 0xc1, 0x52, 0xd2, 0x42, 0x11, 0x00,
    0xcc, 0x01, 0x21, 0xf1, 0x22, 0xda, 0x10, 0x29,
    0x99, 0xf9, 0x02, 0x19, 0x33, 0x01, 0x94, 0x23,
    0x64, 0x11, 0xb0, 0x0a, 0x12, 0x33, 0xfa, 0x01,
    0x92, 0x37, 0x22, 0x71, 0x11, 0x2d, 0xb0, 0xc1,
    0x11, 0x22, 0x3b, 0xf1
  };
  uint32_t        dst[32];
  const char     *slimit = src + 44;
  const uint32_t *dlimit = dst + 32;

  memset(dst, 0xff, 256);

  EXPECT_EQ(2, Unpack11(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(204, dst[0]);

  EXPECT_EQ(3, Unpack11(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);

  EXPECT_EQ(5, Unpack11(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);

  EXPECT_EQ(6, Unpack11(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);

  EXPECT_EQ(7, Unpack11(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);

  EXPECT_EQ(9, Unpack11(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);

  EXPECT_EQ(10, Unpack11(
          src, slimit, dst, dlimit, 7));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);

  EXPECT_EQ(11, Unpack11(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);

  EXPECT_EQ(13, Unpack11(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);

  EXPECT_EQ(14, Unpack11(
          src, slimit, dst, dlimit, 10));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);

  EXPECT_EQ(16, Unpack11(
          src, slimit, dst, dlimit, 11));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);
  EXPECT_EQ(1056, dst[10]);

  EXPECT_EQ(17, Unpack11(
          src, slimit, dst, dlimit, 12));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);
  EXPECT_EQ(1056, dst[10]);
  EXPECT_EQ(665, dst[11]);

  EXPECT_EQ(18, Unpack11(
          src, slimit, dst, dlimit, 13));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);
  EXPECT_EQ(1056, dst[10]);
  EXPECT_EQ(665, dst[11]);
  EXPECT_EQ(1276, dst[12]);

  EXPECT_EQ(20, Unpack11(
          src, slimit, dst, dlimit, 14));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);
  EXPECT_EQ(1056, dst[10]);
  EXPECT_EQ(665, dst[11]);
  EXPECT_EQ(1276, dst[12]);
  EXPECT_EQ(1032, dst[13]);

  EXPECT_EQ(21, Unpack11(
          src, slimit, dst, dlimit, 15));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);
  EXPECT_EQ(1056, dst[10]);
  EXPECT_EQ(665, dst[11]);
  EXPECT_EQ(1276, dst[12]);
  EXPECT_EQ(1032, dst[13]);
  EXPECT_EQ(806, dst[14]);

  EXPECT_EQ(22, Unpack11(
          src, slimit, dst, dlimit, 16));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);
  EXPECT_EQ(1056, dst[10]);
  EXPECT_EQ(665, dst[11]);
  EXPECT_EQ(1276, dst[12]);
  EXPECT_EQ(1032, dst[13]);
  EXPECT_EQ(806, dst[14]);
  EXPECT_EQ(769, dst[15]);

  EXPECT_EQ(24, Unpack11(
          src, slimit, dst, dlimit, 17));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);
  EXPECT_EQ(1056, dst[10]);
  EXPECT_EQ(665, dst[11]);
  EXPECT_EQ(1276, dst[12]);
  EXPECT_EQ(1032, dst[13]);
  EXPECT_EQ(806, dst[14]);
  EXPECT_EQ(769, dst[15]);
  EXPECT_EQ(1185, dst[16]);

  EXPECT_EQ(44, Unpack11(
          src, slimit, dst, dlimit, 32));
  EXPECT_EQ(204, dst[0]);
  EXPECT_EQ(1264, dst[1]);
  EXPECT_EQ(677, dst[2]);
  EXPECT_EQ(1316, dst[3]);
  EXPECT_EQ(264, dst[4]);
  EXPECT_EQ(1027, dst[5]);
  EXPECT_EQ(384, dst[6]);
  EXPECT_EQ(289, dst[7]);
  EXPECT_EQ(1929, dst[8]);
  EXPECT_EQ(182, dst[9]);
  EXPECT_EQ(1056, dst[10]);
  EXPECT_EQ(665, dst[11]);
  EXPECT_EQ(1276, dst[12]);
  EXPECT_EQ(1032, dst[13]);
  EXPECT_EQ(806, dst[14]);
  EXPECT_EQ(769, dst[15]);
  EXPECT_EQ(1185, dst[16]);
  EXPECT_EQ(217, dst[17]);
  EXPECT_EQ(35, dst[18]);
  EXPECT_EQ(768, dst[19]);
  EXPECT_EQ(1289, dst[20]);
  EXPECT_EQ(207, dst[21]);
  EXPECT_EQ(1856, dst[22]);
  EXPECT_EQ(402, dst[23]);
  EXPECT_EQ(441, dst[24]);
  EXPECT_EQ(156, dst[25]);
  EXPECT_EQ(546, dst[26]);
  EXPECT_EQ(731, dst[27]);
  EXPECT_EQ(96, dst[28]);
  EXPECT_EQ(1092, dst[29]);
  EXPECT_EQ(1095, dst[30]);
  EXPECT_EQ(1009, dst[31]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack11(
          src, src, dst, dlimit, 32));
  EXPECT_EQ(-1, Unpack11(
          src, src + 1, dst, dlimit, 32));
  EXPECT_EQ(-1, Unpack11(
          src, slimit, dst, dlimit, 33));
  EXPECT_EQ(-1, Unpack11(
          src, slimit, dst, dst, 32));
}

TEST(Vpacker32, Unpack12) {
  const char      src[12] = {
    0x23, 0x8a, 0xc7, 0xd0, 0xab, 0xc8, 0xe3, 0x03,
    0xaf, 0xd3, 0x93, 0x55
  };
  uint32_t        dst[8];
  const char     *slimit = src + 12;
  const uint32_t *dlimit = dst + 8;

  memset(dst, 0xff, 64);

  EXPECT_EQ(2, Unpack12(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(568, dst[0]);

  EXPECT_EQ(3, Unpack12(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(568, dst[0]);
  EXPECT_EQ(2759, dst[1]);

  EXPECT_EQ(5, Unpack12(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(568, dst[0]);
  EXPECT_EQ(2759, dst[1]);
  EXPECT_EQ(3338, dst[2]);

  EXPECT_EQ(6, Unpack12(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(568, dst[0]);
  EXPECT_EQ(2759, dst[1]);
  EXPECT_EQ(3338, dst[2]);
  EXPECT_EQ(3016, dst[3]);

  EXPECT_EQ(8, Unpack12(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(568, dst[0]);
  EXPECT_EQ(2759, dst[1]);
  EXPECT_EQ(3338, dst[2]);
  EXPECT_EQ(3016, dst[3]);
  EXPECT_EQ(3632, dst[4]);

  EXPECT_EQ(9, Unpack12(
          src, slimit, dst, dlimit, 6));
  EXPECT_EQ(568, dst[0]);
  EXPECT_EQ(2759, dst[1]);
  EXPECT_EQ(3338, dst[2]);
  EXPECT_EQ(3016, dst[3]);
  EXPECT_EQ(3632, dst[4]);
  EXPECT_EQ(943, dst[5]);

  EXPECT_EQ(11, Unpack12(
          src, slimit, dst, dlimit, 7));
  EXPECT_EQ(568, dst[0]);
  EXPECT_EQ(2759, dst[1]);
  EXPECT_EQ(3338, dst[2]);
  EXPECT_EQ(3016, dst[3]);
  EXPECT_EQ(3632, dst[4]);
  EXPECT_EQ(943, dst[5]);
  EXPECT_EQ(3385, dst[6]);

  EXPECT_EQ(12, Unpack12(
          src, slimit, dst, dlimit, 8));
  EXPECT_EQ(568, dst[0]);
  EXPECT_EQ(2759, dst[1]);
  EXPECT_EQ(3338, dst[2]);
  EXPECT_EQ(3016, dst[3]);
  EXPECT_EQ(3632, dst[4]);
  EXPECT_EQ(943, dst[5]);
  EXPECT_EQ(3385, dst[6]);
  EXPECT_EQ(853, dst[7]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack12(
          src, src, dst, dlimit, 8));
  EXPECT_EQ(-1, Unpack12(
          src, src + 1, dst, dlimit, 8));
  EXPECT_EQ(-1, Unpack12(
          src, slimit, dst, dlimit, 9));
  EXPECT_EQ(-1, Unpack12(
          src, slimit, dst, dst, 8));
}

TEST(Vpacker32, Unpack16) {
  const char     *src = "\x23\x8a\xc7\xd0\xab\xc8\xe3\x03";
  uint32_t        dst[4];
  const char     *slimit = src + 8;
  const uint32_t *dlimit = dst + 4;

  memset(dst, 0xff, 32);

  EXPECT_EQ(2, Unpack16(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(9098, dst[0]);

  EXPECT_EQ(4, Unpack16(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(9098, dst[0]);
  EXPECT_EQ(51152, dst[1]);

  EXPECT_EQ(6, Unpack16(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(9098, dst[0]);
  EXPECT_EQ(51152, dst[1]);
  EXPECT_EQ(43976, dst[2]);

  EXPECT_EQ(8, Unpack16(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(9098, dst[0]);
  EXPECT_EQ(51152, dst[1]);
  EXPECT_EQ(43976, dst[2]);
  EXPECT_EQ(58115, dst[3]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack16(
          src, src, dst, dlimit, 4));
  EXPECT_EQ(-1, Unpack16(
          src, src + 1, dst, dlimit, 4));
  EXPECT_EQ(-1, Unpack16(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(-1, Unpack16(
          src, slimit, dst, dst, 4));
}

TEST(Vpacker32, Unpack32) {
  const char      src[16] = {
    0x1e, 0x32, 0xab, 0x4c, 0x93, 0x92, 0xa3, 0xfa,
    0x39, 0x03, 0x23, 0x3a, 0x93, 0xdd, 0xa9, 0x02
  };
  uint32_t        dst[4];
  const char     *slimit = src + 16;
  const uint32_t *dlimit = dst + 4;

  memset(dst, 0xff, 32);

  EXPECT_EQ(4, Unpack32(
          src, slimit, dst, dlimit, 1));
  EXPECT_EQ(506637132, dst[0]);

  EXPECT_EQ(8, Unpack32(
          src, slimit, dst, dlimit, 2));
  EXPECT_EQ(506637132, dst[0]);
  EXPECT_EQ(2475860986, dst[1]);

  EXPECT_EQ(12, Unpack32(
          src, slimit, dst, dlimit, 3));
  EXPECT_EQ(506637132, dst[0]);
  EXPECT_EQ(2475860986, dst[1]);
  EXPECT_EQ(956506938, dst[2]);

  EXPECT_EQ(16, Unpack32(
          src, slimit, dst, dlimit, 4));
  EXPECT_EQ(506637132, dst[0]);
  EXPECT_EQ(2475860986, dst[1]);
  EXPECT_EQ(956506938, dst[2]);
  EXPECT_EQ(2480777474, dst[3]);

  /* Tests for error checks */
  EXPECT_EQ(-1, Unpack32(
          src, src, dst, dlimit, 4));
  EXPECT_EQ(-1, Unpack32(
          src, src + 1, dst, dlimit, 4));
  EXPECT_EQ(-1, Unpack32(
          src, slimit, dst, dlimit, 5));
  EXPECT_EQ(-1, Unpack32(
          src, slimit, dst, dst, 4));
}

TEST(Vpacker32, WriteBits) {
  /* Write 0-bit integers */
  {
    const uint32_t  src[1] = {0};
    char            dst[8];

    EXPECT_EQ(0, WriteBits(
            src, 0, 16, dst, dst));
    EXPECT_EQ(0, WriteBits(
            src, 0, 32, dst, dst));
    EXPECT_EQ(0, WriteBits(
            src, 0, 64, dst, dst));
  }

  /* Write 1-bit integers, and unpack them */
  {
    const uint32_t  src[64] = {
      0, 1, 1, 0, 1, 0, 0, 0,
      1, 1, 0, 0, 1, 1, 1, 0,
      0, 0, 0, 1, 1, 1, 1, 1,
      1, 0, 1, 0, 1, 0, 1, 1,
      0, 0, 1, 1, 1, 0, 0, 1,
      0, 1, 1, 0, 1, 1, 0, 0,
      0, 1, 0, 0, 0, 1, 1, 0,
      0, 0, 0, 0, 1, 1, 1, 1
    };
    uint32_t        buf[64];
    char            dst[8];
    const char     *dlimit = dst + 8;

    memset(dst, 0x00, 8);

    /* Tests for 32 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 1, 32, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack1(
            dst, dlimit, buf, buf + 32, 32));

    for (int i = 0; i < 32; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 64 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 1, 64, dst, dlimit));

    EXPECT_EQ(8, Unpack1(
            dst, dlimit, buf, buf + 64, 64));

    for (int i = 0; i < 64; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 1, 64, dst, dst));
  }

  /* Write 2-bit integers, and unpack them */
  {
    const uint32_t  src[32] = {
      2, 0, 3, 1, 3, 2, 1, 0,
      1, 1, 3, 3, 3, 2, 1, 1,
      3, 1, 2, 2, 1, 0, 0, 2,
      3, 3, 1, 0, 1, 0, 2, 1
    };
    uint32_t        buf[32];
    char            dst[8];
    const char     *dlimit = dst + 8;

    memset(dst, 0x00, 8);

    /* Tests for 16 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 2, 16, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack2(
            dst, dlimit, buf, buf + 16, 16));

    for (int i = 0; i < 16; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 32 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 2, 32, dst, dlimit));

    EXPECT_EQ(8, Unpack2(
            dst, dlimit, buf, buf + 32, 32));

    for (int i = 0; i < 32; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 2, 32, dst, dst));
  }

  /* Write 3-bit integers, and unpack them */
  {
    const uint32_t  src[21] = {
      0, 7, 4, 2, 1, 2, 2, 3,
      1, 3, 2, 2, 6, 5, 1, 7,
      3, 2, 1, 0, 0
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[21 + 3];
    char            dst[8 + 1];
    const char     *dlimit = dst + 9;

    memset(dst, 0x00, 8);

    /* Tests for 16 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 3, 10, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack3(
            dst, dlimit, buf, buf + 16, 10));

    for (int i = 0; i < 10; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 32 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 3, 21, dst, dlimit));

    EXPECT_EQ(8, Unpack3(
            dst, dlimit, buf, buf + 32, 21));

    for (int i = 0; i < 21; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 3, 21, dst, dst));
  }

  /* Write 4-bit integers, and unpack them */
  {
    const uint32_t  src[16] = {
      0, 8, 15, 7, 2, 3, 1, 8,
      11, 3, 4, 1, 9, 7, 4, 1,
    };
    uint32_t        buf[16];
    char            dst[8];
    const char     *dlimit = dst + 8;

    memset(dst, 0x00, 8);

    /* Tests for 8 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 4, 8, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack4(
            dst, dst + 8, buf, buf + 8, 8));

    for (int i = 0; i < 8; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 16 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 4, 16, dst, dlimit));

    EXPECT_EQ(8, Unpack4(
            dst, dst + 8, buf, buf + 16, 16));

    for (int i = 0; i < 16; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 4, 16, dst, dst));
  }

  /* Write 5-bit integers, and unpack them */
  {
    const uint32_t  src[12] = {
      11, 0, 9, 22, 29, 3, 4, 31,
      6, 9, 25, 17
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[12 + 4];
    char            dst[8 + 2];
    const char     *dlimit = dst + 10;

    memset(dst, 0x00, 8);

    /* Tests for 6 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 5, 6, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack5(
            dst, dlimit, buf, buf + 16, 6));

    for (int i = 0; i < 6; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 12 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 5, 12, dst, dlimit));

    EXPECT_EQ(8, Unpack5(
            dst, dlimit, buf, buf + 16, 12));

    for (int i = 0; i < 12; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 5, 16, dst, dst));
  }

  /* Write 6-bit integers, and unpack them */
  {
    const uint32_t  src[10] = {
      28, 8, 12, 21, 63, 54, 38, 49, 0, 61
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[10 + 2];
    char            dst[8 + 1];
    const char     *dlimit = dst + 9;

    memset(dst, 0x00, 8);

    /* Tests for 5 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 6, 5, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack6(
            dst, dlimit, buf, buf + 12, 5));

    for (int i = 0; i < 5; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 10 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 6, 10, dst, dlimit));

    EXPECT_EQ(8, Unpack6(
            dst, dlimit, buf, buf + 12, 10));

    for (int i = 0; i < 10; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 5, 12, dst, dst));
  }

  /* Write 7-bit integers, and unpack them */
  {
    const uint32_t  src[9] = {
      83, 21, 111, 0, 59, 87, 91, 3, 77
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[9 + 7];
    char            dst[8 + 6];
    const char     *dlimit = dst + 14;

    memset(dst, 0x00, 8);

    /* Tests for 4 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 7, 4, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack7(
            dst, dlimit, buf, buf + 16, 4));

    for (int i = 0; i < 4; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 9 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 7, 9, dst, dlimit));

    EXPECT_EQ(8, Unpack7(
            dst, dlimit, buf, buf + 16, 9));

    for (int i = 0; i < 9; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 7, 9, dst, dst));
  }

  /* Write 8-bit integers, and unpack them */
  {
    const uint32_t  src[8] = {
      231, 92, 139, 92, 2, 93, 201, 32
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[8];
    char            dst[8];
    const char     *dlimit = dst + 8;

    memset(dst, 0x00, 8);

    /* Tests for 4 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 8, 4, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack8(
            dst, dst + 8, buf, buf + 8, 4));

    for (int i = 0; i < 4; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 8 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 8, 8, dst, dlimit));

    EXPECT_EQ(8, Unpack8(
            dst, dst + 8, buf, buf + 8, 8));

    for (int i = 0; i < 8; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 8, 8, dst, dst));
  }

  /* Write 9-bit integers, and unpack them */
  {
    const uint32_t  src[6] = {
      328, 29, 193, 291, 382, 499
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[6 + 10];
    char            dst[8 + 10];
    const char     *dlimit = dst + 18;

    memset(dst, 0x00, 8);

    /* Tests for 3 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 9, 3, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack9(
            dst, dlimit, buf, buf + 16, 3));

    for (int i = 0; i < 3; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 6 64-bit integers */
    EXPECT_EQ(7, WriteBits(
            src, 9, 6, dst, dlimit));

    EXPECT_EQ(7, Unpack9(
            dst, dlimit, buf, buf + 16, 6));

    for (int i = 0; i < 6; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 9, 6, dst, dst));
  }

  /* Write 10-bit integers, and unpack them */
  {
    const uint32_t  src[6] = {
      892, 653, 0, 232, 792, 1021
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[6 + 2];
    char            dst[8 + 2];
    const char     *dlimit = dst + 10;

    memset(dst, 0x00, 8);

    /* Tests for 3 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 10, 3, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack10(
            dst, dlimit, buf, buf + 8, 3));

    for (int i = 0; i < 3; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 6 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 10, 6, dst, dlimit));

    EXPECT_EQ(8, Unpack10(
            dst, dlimit, buf, buf + 8, 6));

    for (int i = 0; i < 6; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 10, 6, dst, dst));
  }

  /* Write 11-bit integers, and unpack them */
  {
    const uint32_t  src[5] = {
      2011, 693, 29, 288, 1392
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[6 + 10];
    char            dst[8 + 14];
    const char     *dlimit = dst + 22;

    memset(dst, 0x00, 8);

    /* Tests for 2 64-bit integers */
    EXPECT_EQ(3, WriteBits(
            src, 11, 2, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(3, Unpack11(
            dst, dlimit, buf, buf + 16, 2));

    for (int i = 0; i < 2; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 5 64-bit integers */
    EXPECT_EQ(7, WriteBits(
            src, 11, 5, dst, dlimit));

    EXPECT_EQ(7, Unpack11(
            dst, dlimit, buf, buf + 16, 5));

    for (int i = 0; i < 5; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 11, 5, dst, dst));
  }

  /* Write 12-bit integers, and unpack them */
  {
    const uint32_t  src[4] = {
      3896, 293, 0, 1923
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[6 + 2];
    char            dst[8 + 4];
    const char     *dlimit = dst + 12;

    memset(dst, 0x00, 8);

    /* Tests for 2 64-bit integers */
    EXPECT_EQ(3, WriteBits(
            src, 12, 2, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(3, Unpack12(
            dst, dlimit, buf, buf + 12, 2));

    for (int i = 0; i < 2; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 4 64-bit integers */
    EXPECT_EQ(6, WriteBits(
            src, 12, 4, dst, dlimit));

    EXPECT_EQ(6, Unpack12(
            dst, dlimit, buf, buf + 12, 4));

    for (int i = 0; i < 4; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 12, 4, dst, dst));
  }

  /* Write 16-bit integers, and unpack them */
  {
    const uint32_t  src[4] = {
      63921, 9293, 43921, 192
    };
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[4];
    char            dst[8];
    const char     *dlimit = dst + 8;

    memset(dst, 0x00, 8);

    /* Tests for 2 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 16, 2, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack16(
            dst, dlimit, buf, buf + 4, 2));

    for (int i = 0; i < 2; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for 4 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 16, 4, dst, dlimit));

    EXPECT_EQ(8, Unpack16(
            dst, dlimit, buf, buf + 4, 4));

    for (int i = 0; i < 4; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 16, 4, dst, dst));
  }

  /* Write 32-bit integers, and unpack them */
  {
    const uint32_t  src[2] = {9239201, 392392};
    /*
     * NOTE: +XX responsible for overruns
     * in unpacking functions.
     */
    uint32_t        buf[2];
    char            dst[8];
    const char     *dlimit = dst + 8;

    memset(dst, 0x00, 8);

    /* Tests for 1 64-bit integers */
    EXPECT_EQ(4, WriteBits(
            src, 32, 1, dst, dlimit));

    /* Tests for overruns */
    EXPECT_EQ(0, dst[4]);
    EXPECT_EQ(0, dst[5]);
    EXPECT_EQ(0, dst[6]);
    EXPECT_EQ(0, dst[7]);

    EXPECT_EQ(4, Unpack32(
            dst, dlimit, buf, buf + 1, 1));

    EXPECT_EQ(src[0], buf[0]);

    /* Tests for 2 64-bit integers */
    EXPECT_EQ(8, WriteBits(
            src, 32, 2, dst, dlimit));

    EXPECT_EQ(8, Unpack32(
            dst, dlimit, buf, buf + 2, 2));

    for (int i = 0; i < 2; i++)
      EXPECT_EQ(src[i], buf[i]);

    /* Tests for error checks */
    EXPECT_EQ(-1, WriteBits(
            src, 16, 4, dst, dst));
  }
}

TEST(Vpacker32, ComputePartition) {
  /* -1 is a separator for partitions */
  uint32_t  src[160] = {
    /* 128 integers in a parition */
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, -1,
    /* 128 integers in a parition */
    1, 1, 1, 1, 1, 1, 1, 1, -1,
    /* 4 integers in a parition */
    1, 1, 1, 1, -1,
    /* 6 integers in a parition */
    1, 1, 1, 1, 1, 1, -1,
    /* 7 integers in a parition */
    1, 1, 1, 1, 1, 1, 1, -1,
    /* 2 integers in a parition */
    1, 1
  };

  size_t  parts[11];

  EXPECT_EQ(1, ComputePartition(src, 128, parts));
  EXPECT_EQ(0, parts[0]);
  EXPECT_EQ(128, parts[1]);

  EXPECT_EQ(11, ComputePartition(src, 160, parts));
  EXPECT_EQ(128, parts[1] - parts[0]);
  EXPECT_EQ(1, parts[2] - parts[1]);
  EXPECT_EQ(8, parts[3] - parts[2]);
  EXPECT_EQ(1, parts[4] - parts[3]);
  EXPECT_EQ(4, parts[5] - parts[4]);
  EXPECT_EQ(1, parts[6] - parts[5]);
  EXPECT_EQ(6, parts[7] - parts[6]);
  EXPECT_EQ(1, parts[8] - parts[7]);
  EXPECT_EQ(7, parts[9] - parts[8]);
  EXPECT_EQ(1, parts[10] - parts[9]);
  EXPECT_EQ(2, parts[11] - parts[10]);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
