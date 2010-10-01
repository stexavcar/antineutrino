package org.neutrino.runtime;

import java.util.List;

public class RArray extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("array");

  private final List<RValue> elements;

  public RArray(List<RValue> elements) {
    this.elements = elements;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    StringBuilder buf = new StringBuilder().append("#[");
    for (int i = 0; i < elements.size(); i++) {
      if (i > 0) buf.append(", ");
      buf.append(elements.get(i).toString());
    }
    return buf.append("]").toString();
  }

}
