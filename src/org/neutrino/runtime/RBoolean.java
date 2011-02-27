package org.neutrino.runtime;


public class RBoolean extends RValue {

  private static final RBoolean TRUE = new RBoolean(TypeId.getIds("true"), true);
  private static final RBoolean FALSE = new RBoolean(TypeId.getIds("false"), false);

  private final TypeId[] typeIds;
  private final boolean value;

  private RBoolean(TypeId[] typeIds, boolean value) {
    this.typeIds = typeIds;
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
  public TypeId[] getTypeIds() {
    return typeIds;
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
