package org.neutrino.runtime;

import java.util.List;

public class RReference extends RValue {

  private static final List<TypeId> TYPE_IDS = TypeId.getIds("ref");

  private State state = State.MUTABLE;
  private RValue value;

  public RReference(RValue value) {
    this.value = value;
  }

  @Override
  public List<TypeId> getTypeIds() {
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
