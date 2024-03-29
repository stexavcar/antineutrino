package org.neutrino.runtime;


public class RByteArray extends RAbstractValue implements RPrimitiveArray {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("ByteArray");

  private State state = State.MUTABLE;
  private final byte[] data;

  public RByteArray(byte[] data) {
    this.data = data;
  }

  public RByteArray(int size) {
    this.data = new byte[size];
  }

  @Override
  public RValue get(RValue key) {
    if (key instanceof RInteger) {
      return get(((RInteger) key).value);
    } else {
      return RNull.getInstance();
    }
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  public RValue get(int index) {
    return RInteger.get(data[index]);
  }

  public void set(int key, int value) {
    assert state.allowMutation();
    data[key] = (byte) value;
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
    return state;
  }

}
