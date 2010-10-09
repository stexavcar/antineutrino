package org.neutrino.runtime;

import java.util.List;

public class RPrimitiveArray extends RArray {

  private static final TypeId TYPE_ID = TypeId.get("array");

  private final List<RValue> elements;

  public RPrimitiveArray(List<RValue> elements) {
    this.elements = elements;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  public RValue get(int index) {
    return elements.get(index);
  }

  protected List<RValue> getElements() {
    return elements;
  }

  public int getLength() {
    return elements.size();
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