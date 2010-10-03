package org.neutrino.runtime;

public class RContinuation extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("continuation");

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

}
