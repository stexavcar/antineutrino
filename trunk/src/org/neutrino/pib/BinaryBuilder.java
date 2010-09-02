package org.neutrino.pib;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

public class BinaryBuilder {

  private final Map<String, ModuleBuilder> modules = new TreeMap<String, ModuleBuilder>();

  public ModuleBuilder createModule(String name) {
    ModuleBuilder result = new ModuleBuilder(name);
    modules.put(name, result);
    return result;
  }

  public Binary getResult() {
    Map<String, Module> result = new HashMap<String, Module>();
    for (Map.Entry<String, ModuleBuilder> entry : modules.entrySet()) {
      Module module = entry.getValue().getResult();
      result.put(entry.getKey(), module);
    }
    return new Binary(result);
  }

}
