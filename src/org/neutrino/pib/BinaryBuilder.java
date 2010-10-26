package org.neutrino.pib;

import org.neutrino.compiler.CompilerUniverse;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

public class BinaryBuilder {

  private final CompilerUniverse universe;
  private final Map<String, ModuleBuilder> modules = new TreeMap<String, ModuleBuilder>();

  public BinaryBuilder(CompilerUniverse universe) {
    this.universe = universe;
  }

  public ModuleBuilder createModule(String name) {
    ModuleBuilder result = new ModuleBuilder(universe);
    modules.put(name, result);
    return result;
  }

  public Universe getResult() {
    Map<String, Module> result = new HashMap<String, Module>();
    for (Map.Entry<String, ModuleBuilder> entry : modules.entrySet()) {
      Module module = entry.getValue().getResult();
      result.put(entry.getKey(), module);
    }
    return new Universe(result);
  }

}
