/*-----------------------------------------------------------------------------
 *  Vpacker.cpp - A wrapper of vpacker32/64 for JNI
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2013 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

#include <Vpacker.h>
#include <vpacker32.hpp>
#include <vpacker64.hpp>

#include <limits>

JNIEXPORT jlong JNICALL Java_Vpacker_compress32
    (JNIEnv *env, jobject obj, jintArray src, jbyteArray dst, jlong n) {
  jint  *jsrc = env->GetIntArrayElements(src, NULL);
  jbyte *jdst = env->GetByteArrayElements(dst, NULL);

  /* XXX: Stupid conversion, any other way? */
  uint64_t wsize =
      vpacker32::Compress((uint32_t *)jsrc, (char *)jdst, n);

  env->ReleaseIntArrayElements(src, jsrc, 0);
  env->ReleaseByteArrayElements(dst, jdst, 0);

  return (wsize > std::numeric_limits<int64_t>::max())? 0 : wsize;
}

JNIEXPORT jlong JNICALL Java_Vpacker_compress64
    (JNIEnv *env, jobject obj, jlongArray src, jbyteArray dst, jlong n) {
  jlong *jsrc = env->GetLongArrayElements(src, NULL);
  jbyte *jdst = env->GetByteArrayElements(dst, NULL);

  /* XXX: Stupid conversion, any other way? */
  uint64_t wsize =
      vpacker64::Compress((uint64_t *)jsrc, (char *)jdst, n);

  env->ReleaseLongArrayElements(src, jsrc, 0);
  env->ReleaseByteArrayElements(dst, jdst, 0);

  return (wsize > std::numeric_limits<int64_t>::max())? 0 : wsize;
}

JNIEXPORT jlong JNICALL Java_Vpacker_uncompress32
    (JNIEnv *env, jobject obj, jbyteArray src, jintArray dst, jlong n) {
  jbyte *jsrc = env->GetByteArrayElements(src, NULL);
  jint  *jdst = env->GetIntArrayElements(dst, NULL);

  /* XXX: Stupid conversion, any other way? */
  uint64_t rsize = vpacker32::Uncompress(
      (char *)jsrc, (uint32_t *)jdst, n);

  env->ReleaseByteArrayElements(src, jsrc, 0);
  env->ReleaseIntArrayElements(dst, jdst, 0);

  return (rsize > std::numeric_limits<int64_t>::max())? 0 : rsize;
}

JNIEXPORT jlong JNICALL Java_Vpacker_uncompress64
    (JNIEnv *env, jobject obj, jbyteArray src, jlongArray dst, jlong n) {
  jbyte *jsrc = env->GetByteArrayElements(src, NULL);
  jlong  *jdst = env->GetLongArrayElements(dst, NULL);

  /* XXX: Stupid conversion, any other way? */
  uint64_t rsize = vpacker64::Uncompress(
      (char *)jsrc, (uint64_t *)jdst, n);

  env->ReleaseByteArrayElements(src, jsrc, 0);
  env->ReleaseLongArrayElements(dst, jdst, 0);

  return (rsize > std::numeric_limits<int64_t>::max())? 0 : rsize;
}

JNIEXPORT jlong JNICALL Java_Vpacker_compress32_1bound
    (JNIEnv *env, jobject obj, jlong n) {
  uint64_t rs = vpacker32::CompressBound(n);
  return (rs > std::numeric_limits<int64_t>::max())? 0 : rs;
}

JNIEXPORT jlong JNICALL Java_Vpacker_compress64_1bound
    (JNIEnv *env, jobject obj, jlong n) {
  uint64_t rs = vpacker64::CompressBound(n);
  return (rs > std::numeric_limits<int64_t>::max())? 0 : rs;
}
