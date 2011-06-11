package org.neutrino.runtime;


public abstract class RAbstractValue implements RValue {

  public String toExternalString() {
    return toString();
  }

  public Object toObject() {
    return null;
  }

  public Kind getKind() {
    return null;
  }

  @Override
  public int compareTo(RValue that) {
    throw new UnsupportedOperationException(this.getClass().getSimpleName());
  }

}
