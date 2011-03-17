package org.neutrino.runtime;


public class RNull extends RValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("null");
  private static final RNull INSTANCE = new RNull();

  private RNull() { }

  public static RNull getInstance() {
    return INSTANCE;
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
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
