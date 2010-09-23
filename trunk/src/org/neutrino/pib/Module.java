package org.neutrino.pib;

import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.runtime.RLambda;
import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;

@Growable(Module.TAG)
public class Module implements ISeedable {

  public static final String TAG = "org::neutrino::pib::Module";

  public @SeedMember Map<String, Binding> defs;
  public @SeedMember Map<String, RProtocol> protos;
  public @SeedMember List<RMethod> methods;

  public Module(Map<String, Binding> defs, Map<String, RProtocol> protos,
      List<RMethod> methods) {
    this.defs = defs;
    this.protos = protos;
    this.methods = methods;
  }

  public Module() { }

  public void initialize() {

  }

  public RLambda getEntryPoint() {
    for (Map.Entry<String, Binding> entry : defs.entrySet()) {
      Binding binding = entry.getValue();
      if (binding.getAnnotations().contains("entry_point")) {
        return new RLambda(this, binding.getCode());
      }
    }
    return null;
  }

  @Override
  public String toString() {
    return "a Module {defs: " + defs + ", protos: " + protos + ", methods: "
        + methods + "}";
  }

  public Collection<Binding> getDefinitions() {
    return defs.values();
  }

}
