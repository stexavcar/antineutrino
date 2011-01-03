package org.neutrino.runtime;

public class RByteArray extends RArray {

  private static final TypeId TYPE_ID = TypeId.get("byte_array");

  private final byte[] data;

  public RByteArray(byte[] data) {
    this.data = data;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  public RValue get(int index) {
    return RInteger.get(data[index]);
  }

  public int getByte(int value) {
    return data[value];
  }

  public int getLength() {
    return data.length;
  }

  public byte[] getBytes() {
    return this.data;
  }

  @Override
  public String toString() {
    StringBuilder buf = new StringBuilder().append("#<bytes:");
    for (int i = 0; i < data.length && i < 16; i++) {
      buf.append(' ');
      buf.append(Integer.toHexString((data[i] + 0x100) & 0xFF));
    }
    if (data.length >= 16)
      buf.append(" ...");
    return buf.append(">").toString();
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
