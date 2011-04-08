package org.neutrino.runtime;


public abstract class RValue implements Comparable<RValue> {

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

  /**
   * This enum defines the sort order between values of different types.
   */
  public enum Kind {
    INTEGER, STRING;
  }

  public abstract RProtocol[] getTypeIds();

  public abstract State getState();

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
