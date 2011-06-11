package org.neutrino.runtime;


public class RReference extends RAbstractValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("ref");

  private State state = State.MUTABLE;
  private RValue value;

  public RReference(RValue value) {
    this.value = value;
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
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
