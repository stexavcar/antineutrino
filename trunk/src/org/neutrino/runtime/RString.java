package org.neutrino.runtime;

import org.neutrino.plankton.Store;

public class RString extends RAbstractValue {

  private static RProtocol[] PROTOS;
  public @Store String value;

  public RString(String value) {
    this.value = value;
  }

  public RString() { }

  public String getValue() {
    return value;
  }

  @Override
  public RProtocol[] getTypeIds() {
    if (PROTOS == null)
      PROTOS = RProtocol.getCanonicals("String");
    return PROTOS;
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
  public Object toObject() {
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

  @Override
  public Kind getKind() {
    return Kind.STRING;
  }

  @Override
  public int compareTo(RValue that) {
    if (that instanceof RString) {
      return this.value.compareTo(((RString) that).value);
    } else {
      return this.getKind().compareTo(that.getKind());
    }
  }

}
