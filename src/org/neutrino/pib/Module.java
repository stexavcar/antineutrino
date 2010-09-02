package org.neutrino.pib;

import java.util.Collection;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
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

}
