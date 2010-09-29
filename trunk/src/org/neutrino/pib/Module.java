package org.neutrino.pib;

import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.RLambda;
import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RValue;

@Growable(Module.TAG)
public class Module implements ISeedable {

  public static final String TAG = "org::neutrino::pib::Module";

  public @SeedMember Map<String, Binding> defs;
  public @SeedMember Map<String, RProtocol> protos;
  public @SeedMember List<RMethod> methods;

  private final Map<String, RValue> globals = new HashMap<String, RValue>();

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
      if (binding.getAnnotation("entry_point") != null) {
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

  public RValue getGlobal(String name, Interpreter inter) {
    RValue result = globals.get(name);
    if (result == null) {
      Binding binding = defs.get(name);
      assert binding != null : name;
      result = inter.interpret(this, binding.getCode());
      globals.put(name, result);
    }
    return result;
  }

}