package org.neutrino.runtime;


public interface RValue extends Comparable<RValue> {

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

  public RProtocol[] getTypeIds();

  public State getState();

  public Kind getKind();

  public Object toObject();

  public String toExternalString();

}
