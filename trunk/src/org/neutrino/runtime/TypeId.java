package org.neutrino.runtime;

import java.util.HashMap;
import java.util.Map;

import org.neutrino.plankton.Atomic;
import org.neutrino.plankton.Generator;
import org.neutrino.plankton.Store;

@Atomic
public class TypeId {

  static final String TAG = "org::neutrino::runtime::TypeId";
  private static Map<String, TypeId> namedTypes = new HashMap<String, TypeId>();

  public final @Store String token;

  private TypeId(String token) { this.token = token; }

  @Generator
  public static TypeId get(String name) {
    if (name == null) {
      return new TypeId(null);
    } else {
      TypeId id = namedTypes.get(name);
      if (id == null) {
        id = new TypeId(name);
        namedTypes.put(name, id);
      }
      return id;
    }
  }

  @Override
  public String toString() {
    if (token == null) {
      return "*";
    } else {
      return "%" + token.toString();
    }
  }

}
