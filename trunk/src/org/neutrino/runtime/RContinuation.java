package org.neutrino.runtime;

import java.util.List;

public class RContinuation extends RValue {

  private static final List<TypeId> TYPE_IDS = TypeId.getIds("continuation");

  @Override
  public List<TypeId> getTypeIds() {
    return TYPE_IDS;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
