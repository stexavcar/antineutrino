package org.neutrino.runtime;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.neutrino.plankton.Store;

public class RPrimitiveArray extends RArray {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("array");

  public @Store State state = State.MUTABLE;
  public @Store List<RValue> elements;

  public RPrimitiveArray(List<RValue> elements) {
    this.elements = elements;
  }

  public RPrimitiveArray(int size) {
    this.elements = new ArrayList<RValue>(Collections.nCopies(size, RNull.getInstance()));
  }

  public RPrimitiveArray() { }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  public RValue get(int index) {
    return elements.get(index);
  }

  public void set(int index, RValue value) {
    assert this.state.allowMutation();
    this.getElements().set(index, value);
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

  @Override
  public State getState() {
    return state;
  }

}
