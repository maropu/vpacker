/*-----------------------------------------------------------------------------
 *  vpacker_test.cpp - A synthetic data generator for vpacker
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2013 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <stdint.h>

#include <vector>
#include <algorithm>

namespace {

class Xor128 {
 public:
  Xor128() :
    x(123456789),
    y(362436069),
    z(521288629),
    w(88675123) {}

  ~Xor128() throw() {}

  uint32_t next() {
    uint32_t t = (x ^ (x << 11));
    x = y, y = z, z = w;
    return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
  }

 private:
  uint32_t x, y, z, w;
};

template <class T>
class Rand;

template <>
class Rand<uint32_t> {
 public:
  uint32_t next() {return rv_.next();}
 private:
  Xor128  rv_;
};

template <>
class Rand<uint64_t> {
 public:
  uint64_t next() {
    uint64_t v = rv_.next();
    return (v << 32) | rv_.next();
  }
 private:
  Xor128  rv_;
};

template <class T>
class TestDataMgr {
 public:
  const T *generate(std::vector<T> *d,
                    size_t n, T max) {
    if (d == NULL || n == 0)
      return NULL;

    /* Reset a previsou state */
    d->clear();

    for (size_t i = 0; i < n; i++)
      d->push_back(rv_.next() % max);

    return &(*d)[0];
  }

private:
  Rand<T> rv_;
};

} /* namespace: */
