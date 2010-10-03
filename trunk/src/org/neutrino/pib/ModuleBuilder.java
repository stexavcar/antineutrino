package org.neutrino.pib;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;


public class ModuleBuilder {

  private int nextImplicitProtocolIndex = 0;
  private final Map<String, CodeBuilder> defs = new TreeMap<String, CodeBuilder>();
  private final Map<String, RProtocol> protos = new HashMap<String, RProtocol>();
  private final List<MethodInfo> methods = new ArrayList<MethodInfo>();

  private static class MethodInfo {

    public final String name;
    public final List<Parameter> params;
    public final CodeBuilder builder;

    public MethodInfo(String name, List<Parameter> params,
        CodeBuilder builder) {
      this.name = name;
      this.params = params;
      this.builder = builder;
    }

  }

  public ModuleBuilder(String name) {
  }

  public CodeBuilder createDefinition(Tree.Definition def) {
    CodeBuilder result = new CodeBuilder(def.getAnnotations());
    defs.put(def.getName(), result);
    return result;
  }

  public CodeBuilder createMethod(List<Annotation> annots, String name,
      List<Parameter> params) {
    CodeBuilder result = new CodeBuilder(annots);
    methods.add(new MethodInfo(name, params, result));
    return result;
  }

  public RProtocol createProtocol(List<Annotation> annots, String name) {
    RProtocol proto = new RProtocol(annots, name);
    protos.put(name, proto);
    return proto;
  }

  public RProtocol createImplicitProtocol() {
    int index = nextImplicitProtocolIndex++;
    String name = "implicit-" + index;
    return createProtocol(Collections.<Annotation>emptyList(), name);
  }

  public Module getResult() {
    Map<String, Binding> elms = new HashMap<String, Binding>();
    for (Map.Entry<String, CodeBuilder> entry : defs.entrySet()) {
      CodeBuilder builder = entry.getValue();
      Binding binding = builder.getBinding();
      elms.put(entry.getKey(), binding);
    }
    List<RMethod> methods = new ArrayList<RMethod>();
    for (MethodInfo info : this.methods) {
      CodeBuilder builder = info.builder;
      CodeBundle bundle = builder.getAssembler().getCode();
      methods.add(new RMethod(builder.getAnnotations(), info.name,
          info.params, bundle));
    }
    return new Module(elms, protos, methods);
  }

}
