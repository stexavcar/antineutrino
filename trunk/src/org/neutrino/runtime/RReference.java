package org.neutrino.runtime;

public class RReference extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("ref");

  private RValue value;

  public RReference(RValue value) {
    this.value = value;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  public RValue getValue() {
    return value;
  }

  public void setValue(RValue value) {
    this.value = value;
  }

  @Override
  public String toString() {
    return "#<a Reference: " + value + ">";
  }

}
