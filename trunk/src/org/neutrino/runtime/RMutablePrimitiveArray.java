package org.neutrino.runtime;

import java.util.ArrayList;
import java.util.List;


public class RMutablePrimitiveArray extends RPrimitiveArray {

  private static final List<TypeId> TYPE_IDS = TypeId.getIds("mutarr");

  public RMutablePrimitiveArray(int size) {
    super(createBackingStore(size));
  }

  private static List<RValue> createBackingStore(int size) {
    List<RValue> result = new ArrayList<RValue>();
    for (int i = 0; i < size; i++)
      result.add(RNull.getInstance());
    return result;
  }

  public void set(int index, RValue value) {
    this.getElements().set(index, value);
  }

  @Override
  public List<TypeId> getTypeIds() {
    return TYPE_IDS;
  }

}
