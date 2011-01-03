package org.neutrino.runtime;

public abstract class RValue {

  public enum State {
    UNDER_CONSTRUCTION, MUTABLE, IMMUTABLE
  }

  public abstract TypeId getTypeId();

  public abstract State getState();

  public String toExternalString() {
    return toString();
  }

}
