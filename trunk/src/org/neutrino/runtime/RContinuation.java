package org.neutrino.runtime;


public class RContinuation extends RValue {

  private static final TypeId[] TYPE_IDS = TypeId.getIds("continuation");

  @Override
  public TypeId[] getTypeIds() {
    return TYPE_IDS;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
