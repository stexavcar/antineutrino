package org.neutrino.runtime;


public class RReference extends RValue {

  private static final TypeId[] TYPE_IDS = TypeId.getIds("ref");

  private State state = State.MUTABLE;
  private RValue value;

  public RReference(RValue value) {
    this.value = value;
  }

  @Override
  public TypeId[] getTypeIds() {
    return TYPE_IDS;
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

  @Override
  public State getState() {
    return state;
  }

}
