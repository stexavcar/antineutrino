package org.neutrino.runtime;

public class RBoolean extends RValue {

  private static final RBoolean TRUE = new RBoolean(TypeId.get("true"), true);
  private static final RBoolean FALSE = new RBoolean(TypeId.get("false"), false);

  private final TypeId typeId;
  private final boolean value;

  private RBoolean(TypeId typeId, boolean value) {
    this.typeId = typeId;
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
  public TypeId getTypeId() {
    return typeId;
  }

  @Override
  public String toString() {
    return value ? "#t" : "#f";
  }

}
