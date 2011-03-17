package org.neutrino.runtime;


public class RBoolean extends RValue {

  private static final RBoolean TRUE = new RBoolean(RProtocol.getCanonicals("True"), true);
  private static final RBoolean FALSE = new RBoolean(RProtocol.getCanonicals("False"), false);

  private final RProtocol[] protos;
  private final boolean value;

  private RBoolean(RProtocol[] protos, boolean value) {
    this.protos = protos;
    this.value = value;
  }

  public static RBoolean getTrue() {
    return TRUE;
  }

  public static RBoolean getFalse() {
    return FALSE;
  }

  public boolean getValue() {
    return value;
  }

  public static RValue get(boolean b) {
    return b ? TRUE : FALSE;
  }

  @Override
  public RProtocol[] getTypeIds() {
    return protos;
  }

  @Override
  public String toString() {
    return value ? "#t" : "#f";
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
