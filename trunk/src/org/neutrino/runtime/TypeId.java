package org.neutrino.runtime;

import java.util.HashMap;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.PMap;
import org.neutrino.plankton.PString;
import org.neutrino.plankton.PValue;
import org.neutrino.plankton.Plankton;
import org.neutrino.plankton.annotations.Factory;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(TypeId.TAG)
public class TypeId implements ISeedable {

  static final String TAG = "org::neutrino::runtime::TypeId";
  private static Map<String, TypeId> namedTypes = new HashMap<String, TypeId>();

  public @SeedMember String token;

  private TypeId(String token) { this.token = token; }

  public static TypeId get(String name) {
    TypeId id = namedTypes.get(name);
    if (id == null) {
      id = new TypeId(name);
      namedTypes.put(name, id);
    }
    return id;
  }

  private static final PString TOKEN = Plankton.newString("token");
  public static @Factory TypeId grow(PValue payload) {
    return get(((PString) ((PMap) payload).get(TOKEN, null)).getValue());
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
