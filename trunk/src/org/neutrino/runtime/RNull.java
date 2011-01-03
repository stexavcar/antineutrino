package org.neutrino.runtime;

public class RNull extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("null");
  private static final RNull INSTANCE = new RNull();

  private RNull() { }

  public static RNull getInstance() {
    return INSTANCE;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return "#n";
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
