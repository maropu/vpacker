/*-----------------------------------------------------------------------------
 *  Vpacker.java - A wrapper of vpacker32/64 for JNI
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2013 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

public class Vpacker {
  /*-------------------------------------------------
   * A interface for compression, a input 32-bit or
   * 64-bit array in *src is compressed, and output
   * to *dst as a byte sequence. Notice that the
   * compress functions is assumed to encode a
   * sequence of 'positive' integers with high
   * skewness. Therefore, negative integers could
   * deteriorate compression ratios.
   *
   * src    : input buffer
   * dst    : output buffer
   * n      : # of input integers
   * return : # of written bytes, or 0 if it fails
   *-------------------------------------------------
   */
  public native static long
      compress32(final int[] src, byte[] dst, long n);
  public native static long
      compress64(final long[] src, byte[] dst, long n);


  /*-------------------------------------------------
   * A interface for decompression, a input byte
   * sequence compressed by vpacker32/64_compress()
   * is decompressed, and output to *dst as a 32-bit
   * or 64-bit array.
   *
   * src    : input buffer
   * dst    : output buffer
   * n      : # of input bytes
   * return : # of read bytes, or 0 if it fails
   *-------------------------------------------------
   */
  public native static long
      uncompress32(final byte[] src, int[] dst, long n);
  public native static long
      uncompress64(final byte[] src, long[] dst, long n);


  /*-------------------------------------------------
   * The function provides the maximumx size that
   * vpacker32/64_compress() may output. It is useful
   * to know the size in advance because of memory
   * allocation for *dst during compression. These
   * functions returns 0 if the size is beyound
   * supported ones for vpacker.
   *
   *  n      : # of input integers
   *  return : maximum size, or 0 if unsupported
   *-------------------------------------------------
   */
  public native static long
      compress32_bound(long n);
  public native static long
      compress64_bound(long n);
}
