package org.neutrino.runtime;

public class RBoolean extends RValue {

  private static final RBoolean TRUE = new RBoolean(true);
  private static final RBoolean FALSE = new RBoolean(false);

  private final boolean value;

  private RBoolean(boolean value) {
    this.value = value;
  }

  public static RBoolean getTrue() {
    return TRUE;
  }

  public static RBoolean getFalse() {
    return FALSE;
  }

  @Override
  public String toString() {
    return value ? "#t" : "#f";
  }

}
