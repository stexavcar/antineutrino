package org.neutrino.runtime;

import java.util.Map;

public class RObject extends RValue {

  private State state = State.UNDER_CONSTRUCTION;
  private final RProtocol proto;
  private final Map<RFieldKey, RValue> outer;

  public RObject(RProtocol proto, Map<RFieldKey, RValue> outer) {
    this.proto = proto;
    this.outer = outer;
  }

  public RValue getField(RFieldKey field) {
    return outer.get(field);
  }

  @Override
  public TypeId getTypeId() {
    return proto.getInstanceTypeId();
  }

  @Override
  public String toString() {
    return "#<an Object: " + proto.getName() + ">";
  }

  @Override
  public State getState() {
    return state;
  }

}
