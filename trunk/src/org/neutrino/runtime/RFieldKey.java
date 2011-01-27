package org.neutrino.runtime;

import org.neutrino.plankton.Store;

public class RFieldKey extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("FieldKey");
  public @Store Object id;

  public RFieldKey(Object id) {
    this.id = id;
  }

  public RFieldKey() { }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
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
