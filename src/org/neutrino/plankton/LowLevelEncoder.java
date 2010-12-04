package org.neutrino.plankton;

import java.io.IOException;
import java.io.OutputStream;

public class LowLevelEncoder {

  static final int kMaxByteCount = 8;
  static final int kMaxEncodedSize = 9;
  static final int kSmallLimit = 16;

  private final OutputStream out;
  private int writtenCount = 0;
  private boolean atHalf = false;
  private int encodedCursor = 0;
  private final byte[] encoded = new byte[kMaxEncodedSize];
  private final int[] longBuffer = new int[10];

  public LowLevelEncoder(OutputStream out) {
    this.out = out;
  }

  public void write(int b) throws IOException {
    if (writtenCount == kMaxByteCount)
      flush();
    if (b < kSmallLimit) {
      if (atHalf) {
        encoded[encodedCursor] |= b & 0x0F;
        atHalf = false;
      } else {
        encoded[++encodedCursor] = (byte) (b << 4);
        atHalf = true;
      }
    } else {
      b -= kSmallLimit;
      encoded[0] |= (128 >> writtenCount);
      if (atHalf) {
        encoded[encodedCursor] |= (byte) ((b >> 4) & 0x0F);
        encoded[++encodedCursor] = (byte) (b << 4);
      } else {
        encoded[++encodedCursor] = (byte) b;
      }
    }
    writtenCount++;
  }

  public void flush() throws IOException {
    if (writtenCount == 0) {
      return;
    }
    while (writtenCount < kMaxByteCount)
      write((byte) 0);
    out.write(encoded, 0, encodedCursor + 1);
    encoded[0] = 0;
    encodedCursor = 0;
    atHalf = false;
    writtenCount = 0;
  }

  public void writeUnsigned(long value) throws IOException {
    assert value >= 0;
    int i = 0;
    do {
      int low = (int) (value & 0x7F);
      value >>>= 7;
      int part = (low << 1);
      longBuffer[i] = part;
      i++;
    } while (value != 0);
    while (i > 1)
      write(longBuffer[--i]);
    write(longBuffer[0] | 1);
  }

  public void writeSigned(long value) throws IOException {
    writeUnsigned((value << 1) ^ (value >> 63));
  }

}
