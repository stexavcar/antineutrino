package org.neutrino.runtime;

public class RMutableByteArray extends RByteArray {

  private static final TypeId TYPE_ID = TypeId.get("mutbytarr");

  public RMutableByteArray(int size) {
    super(new byte[size]);
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  public void set(int key, int value) {
    this.getBytes()[key] = (byte) value;
  }

}
