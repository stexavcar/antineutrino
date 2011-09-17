package org.neutrino.runtime;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.javatrino.ast.Method;
import org.javatrino.utils.Utils;
import org.neutrino.plankton.Store;

public class RObject extends RAbstractValue {

  private static final RImpl emptyImpl = new RImpl();

  public @Store State state = State.UNDER_CONSTRUCTION;
  public @Store Map<RFieldKey, RValue> fields;
  public @Store RImpl impl = emptyImpl;

  public RObject(RProtocol proto, Map<RFieldKey, RValue> outer) {
    this.fields = outer;
    this.impl = this.impl.addProtocol(proto);
  }

  public RObject() {
    this.fields = new HashMap<RFieldKey, RValue>();
  }

  public RImpl getImpl() {
    return this.impl;
  }

  public RValue getField(RFieldKey field) {
    return fields.get(field);
  }

  public void setField(RFieldKey field, RValue value) {
    assert state != State.IMMUTABLE;
    fields.put(field, value);
  }

  public void addIntrinsics(List<Method> values) {
    this.impl = this.impl.addIntrinsics(values);
  }

  public void addProtocol(RProtocol proto) {
    this.impl = this.impl.addProtocol(proto);
  }

  @Override
  public RProtocol[] getTypeIds() {
    return impl.getProtocolArray();
  }

  @Override
  public String toString() {
    List<RProtocol> protocols = impl.getProtocols();
    if (protocols.isEmpty()) {
      return "#<an Object>";
    } else if (protocols.size() == 1) {
      RProtocol protocol = protocols.get(0);
      return "#<an " + protocol.getId().toExternalString() + ">";
    } else {
      return "#<an Object: " + Utils.join(protocols, ", ") + ">";
    }
  }

  @Override
  public State getState() {
    return state;
  }

}
