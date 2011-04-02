package org.neutrino.runtime;

public class RFunction extends RValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("Function");

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
