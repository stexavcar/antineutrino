package org.neutrino.runtime;

import java.util.List;

public class RMutableByteArray extends RByteArray {

  private static final List<TypeId> TYPE_IDS = TypeId.getIds("mutbytarr");

  public RMutableByteArray(int size) {
    super(new byte[size]);
  }

  @Override
  public List<TypeId> getTypeIds() {
    return TYPE_IDS;
  }

  public void set(int key, int value) {
    this.getBytes()[key] = (byte) value;
  }

}
