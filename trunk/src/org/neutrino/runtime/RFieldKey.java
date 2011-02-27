package org.neutrino.runtime;

import java.util.List;

import org.neutrino.plankton.Store;

public class RFieldKey extends RValue {

  private static final List<TypeId> TYPE_IDS = TypeId.getIds("FieldKey");
  public @Store Object id;

  public RFieldKey(Object id) {
    this.id = id;
  }

  public RFieldKey() { }

  @Override
  public List<TypeId> getTypeIds() {
    return TYPE_IDS;
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
