package org.neutrino.runtime;

import java.util.HashMap;
import java.util.Map;

public class TypeId {

  private static Map<String, TypeId> namedTypes = new HashMap<String, TypeId>();

  private TypeId() { }

  public static TypeId get(String name) {
    TypeId id = namedTypes.get(name);
    if (id == null) {
      id = new TypeId();
      namedTypes.put(name, id);
    }
    return id;
  }

  public static TypeId anonymous() {
    return new TypeId();
  }

}
