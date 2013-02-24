/*-----------------------------------------------------------------------------
 *  SampleVpacker.java - A naive sample code for vpacker
 *
 *  Coding-Style: google-styleguide
 *      https://code.google.com/p/google-styleguide/
 *
 *  Copyright 2013 Takeshi Yamamuro <linguin.m.s_at_gmail.com>
 *-----------------------------------------------------------------------------
 */

import java.io.*;

class TestVpacker {
  static {System.loadLibrary("vpackj");}

  public static void main(String args[]) throws Exception {
    /* Ready for a input buffer */
    final int[] src = {
      92, 923, 0, 31, 293, 92, 22, 38, 23, 18, 102, 231
    };

    /* Ready for a output buffer */
    int[]   buf = new int[12];
    byte[]  dst = new byte[
        (int)Vpacker.compress32_bound(12)];

    long wsize = Vpacker.compress32(src, dst, 12);
    if (wsize == 0)
      throw new Exception("Exception: Vpacker.compress32");

    long rsize = Vpacker.uncompress32(dst, buf, 12);
    if (wsize != rsize)
      throw new Exception("Exception: Vpacker.uncompress32");

    for (int i = 0; i < 12; i++)
      System.out.println(i + " : " + src[i] + " == " + buf[i] + "?");
  }
}
