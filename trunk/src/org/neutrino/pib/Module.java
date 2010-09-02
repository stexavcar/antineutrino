package org.neutrino.pib;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PString;
import org.neutrino.plankton.Plankton;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(Module.TAG)
public class Module implements ISeedable {

  public static final String TAG = "org::neutrino::pib::Module";

  public @SeedMember Map<String, Binding> defs;

  public Module(Map<String, Binding> defs) {
    this.defs = defs;
  }

  public Module() { }

  @Override
  public String toString() {
    return "a Module " + defs;
  }

  public Collection<Binding> getDefinitions() {
    return defs.values();
  }

  public PSeed toSeed(Plankton plankton) {
    Map<PString, PSeed> map = new HashMap<PString, PSeed>();
    for (Map.Entry<String, Binding> entry : defs.entrySet()) {
      PSeed value = entry.getValue().toSeed(plankton);
      map.put(Plankton.newString(entry.getKey()), value);
    }
    return plankton.newSeed(TAG, Plankton.newMap(map));
  }

}
