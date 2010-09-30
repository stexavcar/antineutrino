package org.neutrino.runtime;

public class RInteger extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("Integer");

  private final int value;

  public RInteger(int value) {
    this.value = value;
  }

  public int getValue() {
    return value;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return Integer.toString(value);
  }

}
