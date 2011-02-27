package org.neutrino.runtime;


public class RMutableByteArray extends RByteArray {

  private static final TypeId[] TYPE_IDS = TypeId.getIds("mutbytarr");

  public RMutableByteArray(int size) {
    super(new byte[size]);
  }

  @Override
  public TypeId[] getTypeIds() {
    return TYPE_IDS;
  }

  public void set(int key, int value) {
    this.getBytes()[key] = (byte) value;
  }

}
