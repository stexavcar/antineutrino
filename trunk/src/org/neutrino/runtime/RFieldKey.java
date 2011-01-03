package org.neutrino.runtime;

public class RFieldKey extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("FieldKey");

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
