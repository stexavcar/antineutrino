package org.neutrino.runtime;


public abstract class RValue {

  public enum State {
    UNDER_CONSTRUCTION, MUTABLE, IMMUTABLE
  }

  public abstract RProtocol[] getTypeIds();

  public abstract State getState();

  public String toExternalString() {
    return toString();
  }

  public Object toObject() {
    return null;
  }

}
