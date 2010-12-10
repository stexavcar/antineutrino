package org.neutrino.runtime;

import java.util.HashMap;
import java.util.Map;

import org.neutrino.oldton.ISeedable;
import org.neutrino.oldton.annotations.Growable;
import org.neutrino.oldton.annotations.SeedMember;
import org.neutrino.plankton.Atomic;
import org.neutrino.plankton.Generator;
import org.neutrino.plankton.Store;

@Atomic
@Growable(TypeId.TAG)
public class TypeId implements ISeedable {

  static final String TAG = "org::neutrino::runtime::TypeId";
  private static Map<String, TypeId> namedTypes = new HashMap<String, TypeId>();

  public final @Store @SeedMember String token;

  private TypeId(String token) { this.token = token; }

  @Generator
  public static TypeId get(String name) {
    TypeId id = namedTypes.get(name);
    if (id == null) {
      id = new TypeId(name);
      namedTypes.put(name, id);
    }
    return id;
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
