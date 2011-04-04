package org.neutrino.runtime;


public abstract class RValue {

  public enum State {

    UNDER_CONSTRUCTION(true), MUTABLE(true), IMMUTABLE(false);

    private final boolean allowMutation;

    private State(boolean allowMutation) {
      this.allowMutation = allowMutation;
    }

    public boolean allowMutation() {
      return this.allowMutation;
    }

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
