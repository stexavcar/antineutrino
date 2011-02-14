package org.neutrino.pib;

import static org.javatrino.ast.Expression.StaticFactory.eConstant;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.neutrino.compiler.CompilerUniverse;
import org.neutrino.compiler.Source;
import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;


public class ModuleBuilder {

  private final CompilerUniverse universe;
  private static int nextImplicitProtocolIndex = 0;
  private final Map<String, CodeBuilder> defs = new TreeMap<String, CodeBuilder>();
  private final Map<String, RProtocol> protos = new HashMap<String, RProtocol>();
  private final List<MethodInfo> methods = new ArrayList<MethodInfo>();
  private final Map<String, List<String>> inheritance = new HashMap<String, List<String>>();

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

  public ModuleBuilder(CompilerUniverse universe) {
    this.universe = universe;
  }

  public CompilerUniverse getUniverse() {
    return universe;
  }

  public void declareInheritance(String sub, String shuper) {
    List<String> locals = inheritance.get(sub);
    if (locals == null) {
      locals = new ArrayList<String>();
      inheritance.put(sub, locals);
    }
    locals.add(shuper);
  }

  public CodeBuilder createDefinition(Source origin, Tree.Definition def) {
    CodeBuilder result = new CodeBuilder(origin, def.getAnnotations());
    defs.put(def.getName(), result);
    return result;
  }

  public CodeBuilder createMethod(Source origin, List<Annotation> annots, String name,
      List<Parameter> params) {
    CodeBuilder result = new CodeBuilder(origin, annots);
    methods.add(new MethodInfo(name, params, result));
    return result;
  }

  public RProtocol createProtocol(Source origin, List<Annotation> annots, String id,
      String displayName) {
    RProtocol proto = new RProtocol(annots, id, displayName);
    protos.put(id, proto);
    CodeBuilder builder = new CodeBuilder(origin, Collections.<Annotation>emptyList());
    defs.put(id, builder);
    builder.getAssembler().push(proto);
    builder.getAssembler().rethurn();
    builder.getAssembler().finalize(eConstant(proto), null);
    return proto;
  }

  public RProtocol createImplicitProtocol(Source origin, String displayName) {
    int index = nextImplicitProtocolIndex++;
    String id = "implicit-" + index;
    return createProtocol(origin, Collections.<Annotation>emptyList(), id,
        displayName);
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
    return new Module(elms, protos, methods, inheritance);
  }

}
