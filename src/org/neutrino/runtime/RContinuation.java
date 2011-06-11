package org.neutrino.runtime;


public class RContinuation extends RAbstractValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("continuation");

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
