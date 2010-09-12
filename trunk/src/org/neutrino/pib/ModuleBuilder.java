package org.neutrino.pib;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;


public class ModuleBuilder {

  private final String name;
  private final Map<String, DefinitionBuilder> defs = new TreeMap<String, DefinitionBuilder>();

  public ModuleBuilder(String name) {
    this.name = name;
  }

  public DefinitionBuilder createDefinition(String name, List<String> annots) {
    DefinitionBuilder result = new DefinitionBuilder(annots);
    defs.put(name, result);
    return result;
  }

  public Module getResult() {
    Map<String, Binding> elms = new HashMap<String, Binding>();
    for (Map.Entry<String, DefinitionBuilder> entry : defs.entrySet()) {
      DefinitionBuilder builder = entry.getValue();
      Binding binding = builder.getBinding();
      elms.put(entry.getKey(), binding);
    }
    return new Module(elms);
  }

}
