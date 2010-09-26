package org.neutrino.pib;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;


public class ModuleBuilder {

  private final Map<String, CodeBuilder<Tree.Definition>> defs = new TreeMap<String, CodeBuilder<Tree.Definition>>();
  private final Map<String, RProtocol> protos = new HashMap<String, RProtocol>();
  private final List<CodeBuilder<Tree.Method>> methods = new ArrayList<CodeBuilder<Tree.Method>>();

  public ModuleBuilder(String name) {
  }

  public CodeBuilder<?> createDefinition(Tree.Definition def) {
    CodeBuilder<Tree.Definition> result = new CodeBuilder<Tree.Definition>(def);
    defs.put(def.getName(), result);
    return result;
  }

  public CodeBuilder<?> createMethod(Tree.Method method) {
    CodeBuilder<Tree.Method> result = new CodeBuilder<Tree.Method>(method);
    methods.add(result);
    return result;
  }

  public void createProtocol(List<Annotation> annots, String name) {
    RProtocol proto = new RProtocol(annots, name);
    protos.put(name, proto);
  }

  public Module getResult() {
    Map<String, Binding> elms = new HashMap<String, Binding>();
    for (Map.Entry<String, CodeBuilder<Tree.Definition>> entry : defs.entrySet()) {
      CodeBuilder<Tree.Definition> builder = entry.getValue();
      Binding binding = builder.getBinding();
      elms.put(entry.getKey(), binding);
    }
    List<RMethod> methods = new ArrayList<RMethod>();
    for (CodeBuilder<Tree.Method> builder : this.methods) {
      Tree.Method method = builder.getSyntax();
      CodeBundle bundle = builder.getAssembler().getCode();
      methods.add(new RMethod(method.getName(), method.getParameters(), bundle));
    }
    return new Module(elms, protos, methods);
  }

}
