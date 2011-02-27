package org.neutrino.runtime;

import org.neutrino.plankton.Store;

public class RString extends RValue {

  private static final TypeId[] TYPE_IDS = TypeId.getIds("str");

  public @Store String value;

  public RString(String value) {
    this.value = value;
  }

  public RString() { }

  public String getValue() {
    return value;
  }

  @Override
  public TypeId[] getTypeIds() {
    return TYPE_IDS;
  }

  @Override
  public String toString() {
    return "#'" + value + "'";
  }

  @Override
  public String toExternalString() {
    return value;
  }

  @Override
  public int hashCode() {
    return value.hashCode();
  }

  @Override
  public boolean equals(Object obj) {
    return (obj instanceof RString) && ((RString) obj).value.equals(this.value);
  }

  public static RString of(String value) {
    return new RString(value);
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
