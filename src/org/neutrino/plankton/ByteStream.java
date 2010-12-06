package org.neutrino.plankton;

public class ByteStream {

  private static final int kInitialCapacity = 64;
  static final int kMaxEncodedSize = 9;

  byte[] data = new byte[kInitialCapacity];
  private int cursor = 0;

  public void add(byte[] bytes, int length) {
    if (cursor + length >= data.length)
      ensureCapacity(cursor + length);
    for (int i = 0; i < length; i++)
      data[cursor + i] = bytes[i];
    cursor += length;
  }

  private void ensureCapacity(int length) {
    data = copyStore(2 * length);
  }

  public byte[] getBytes() {
    return copyStore(cursor);
  }

  public int length() {
    return cursor;
  }

  private byte[] copyStore(int length) {
    byte[] result = new byte[length];
    for (int i = 0; i < this.cursor; i++)
      result[i] = data[i];
    return result;
  }

}
