package org.neutrino.runtime;

import java.util.HashMap;
import java.util.Map;

import org.neutrino.oldton.ISeedable;
import org.neutrino.oldton.PMap;
import org.neutrino.oldton.PString;
import org.neutrino.oldton.PValue;
import org.neutrino.oldton.Oldton;
import org.neutrino.oldton.annotations.Factory;
import org.neutrino.oldton.annotations.Growable;
import org.neutrino.oldton.annotations.SeedMember;

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

  private static final PString TOKEN = Oldton.newString("token");
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
