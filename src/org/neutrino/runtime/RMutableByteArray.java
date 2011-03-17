package org.neutrino.runtime;


public class RMutableByteArray extends RByteArray {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("mutbytarr");

  public RMutableByteArray(int size) {
    super(new byte[size]);
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  public void set(int key, int value) {
    this.getBytes()[key] = (byte) value;
  }

}
