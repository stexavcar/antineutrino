package org.neutrino.runtime;

import java.util.Collections;

public abstract class RValue {

  public enum State {
    UNDER_CONSTRUCTION, MUTABLE, IMMUTABLE
  }

  public abstract TypeId getTypeId();

  public Iterable<TypeId> getTypeIds() {
    return Collections.singleton(getTypeId());
  }

  public abstract State getState();

  public String toExternalString() {
    return toString();
  }

}
