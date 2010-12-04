package org.neutrino.plankton;

import java.io.IOException;
import java.io.InputStream;

public class LowLevelDecoder {

  private final InputStream in;

  private int cursor = LowLevelEncoder.kMaxByteCount;
  private final int[] buffer = new int[LowLevelEncoder.kMaxByteCount];

  public LowLevelDecoder(InputStream in) {
    this.in = in;
  }

  public int read() throws IOException {
    if (cursor == LowLevelEncoder.kMaxByteCount)
      fillBuffer();
    return buffer[cursor++];
  }

  private void fillBuffer() throws IOException {
    int head = in.read();
    int current = 0;
    boolean atHalf = false;
    for (int i = 0; i < LowLevelEncoder.kMaxByteCount; i++) {
      boolean isHalf = (head & (128 >> i)) == 0;
      if (isHalf) {
        if (atHalf) {
          buffer[i] = (byte) (current & 0x0F);
          atHalf = false;
        } else {
          current = in.read();
          buffer[i] = (byte) ((current >> 4) & 0x0F);
          atHalf = true;
        }
      } else {
        if (atHalf) {
          int value = (current & 0x0F) << 4;
          current = in.read();
          buffer[i] = (value | ((current >>> 4) & 0x0F)) + LowLevelEncoder.kSmallLimit;
        } else {
          buffer[i] = in.read() + LowLevelEncoder.kSmallLimit;
        }
      }
    }
    cursor = 0;
  }

  public long readUnsigned() throws IOException {
    long result = 0;
    while (true) {
      int next = read();
      result = (result << 7) | (next >> 1);
      if ((next & 1) == 1)
        return result;
    }
  }

  public long readSigned() throws IOException {
    long coded = readUnsigned();
    if ((coded & 1) == 0) {
      return coded >>> 1;
    } else {
      return -(coded >>> 1) - 1;
    }
  }

}
