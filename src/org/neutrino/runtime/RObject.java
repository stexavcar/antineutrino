package org.neutrino.runtime;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.javatrino.ast.Method;

public class RObject extends RValue {

  private State state = State.UNDER_CONSTRUCTION;
  private final Map<RFieldKey, RValue> outer;
  private final List<RProtocol> protos = new ArrayList<RProtocol>();
  private List<TypeId> typeIds = Collections.<TypeId>emptyList();
  private final List<Method> intrinsics = new ArrayList<Method>();

  public RObject(RProtocol proto, Map<RFieldKey, RValue> outer) {
    this.outer = outer;
    this.protos.add(proto);
  }

  public RObject() {
    this.outer = new HashMap<RFieldKey, RValue>();
  }

  public List<Method> getIntrinsics() {
    return this.intrinsics;
  }

  public RValue getField(RFieldKey field) {
    return outer.get(field);
  }

  public void setField(RFieldKey field, RValue value) {
    assert state != State.IMMUTABLE;
    outer.put(field, value);
  }

  public void addIntrinsics(List<Method> values) {
    intrinsics.addAll(values);
  }

  public void addProtocol(RProtocol proto) {
    this.typeIds = null;
    this.protos.add(proto);
  }

  @Override
  public TypeId getTypeId() {
    return protos.get(0).getInstanceTypeId();
  }

  @Override
  public Iterable<TypeId> getTypeIds() {
    if (this.typeIds == null) {
      typeIds = new ArrayList<TypeId>();
      for (RProtocol proto : protos)
        typeIds.add(proto.getInstanceTypeId());
    }
    return this.typeIds;
  }

  @Override
  public String toString() {
    return "#<an Object: " + protos + ">";
  }

  @Override
  public State getState() {
    return state;
  }

}
