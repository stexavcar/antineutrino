package org.neutrino.runtime;

import java.util.Map;

public class RDictionary extends RAbstractValue implements RPrimitiveMap {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("PrimitiveMap");
  private final Map<RValue, RValue> values;

  public RDictionary(Map<RValue, RValue> values) {
    this.values = values;
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

  @Override
  public RValue get(RValue key) {
    RValue result = values.get(key);
    return result == null ? RNull.getInstance() : result;
  }

}
