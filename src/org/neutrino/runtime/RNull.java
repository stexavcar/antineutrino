package org.neutrino.runtime;

import java.util.List;

public class RNull extends RValue {

  private static final List<TypeId> TYPE_IDS = TypeId.getIds("null");
  private static final RNull INSTANCE = new RNull();

  private RNull() { }

  public static RNull getInstance() {
    return INSTANCE;
  }

  @Override
  public List<TypeId> getTypeIds() {
    return TYPE_IDS;
  }

  @Override
  public String toString() {
    return "#n";
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
