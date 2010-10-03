package org.neutrino.runtime;

public class RObject extends RValue {

  private final RProtocol proto;
  private final RValue[] outer;

  public RObject(RProtocol proto, RValue[] outer) {
    this.proto = proto;
    this.outer = outer;
  }

  public RValue getField(int index) {
    return outer[index];
  }

  @Override
  public TypeId getTypeId() {
    return proto.getInstanceTypeId();
  }

  @Override
  public String toString() {
    return "#<an Object: " + getTypeId() + ">";
  }

}
