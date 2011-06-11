package org.neutrino.runtime;

import org.neutrino.plankton.Store;

public class RFieldKey extends RAbstractValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("FieldKey");
  public @Store Object id;

  public RFieldKey(Object id) {
    this.id = id;
  }

  public RFieldKey() { }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

  @Override
  public String toString() {
    return "field_key(" + id + ")";
  }

}
