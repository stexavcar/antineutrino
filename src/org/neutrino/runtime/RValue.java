package org.neutrino.runtime;

public abstract class RValue {

  public abstract TypeId getTypeId();

  public String toExternalString() {
    return toString();
  }

}
