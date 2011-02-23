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


public class ModuleBuilder {

  private final Map<String, RProtocol> protos;
  private final Map<String, Binding> defs;
  private final List<RMethod> methods;
  private final Map<String, List<String>> rawInheritance = new HashMap<String, List<String>>();

  private final CompilerUniverse universe;
  private static int nextImplicitProtocolIndex = 0;

  public ModuleBuilder(CompilerUniverse universe) {
    this.universe = universe;
    this.methods = new ArrayList<RMethod>();
    this.protos = new HashMap<String, RProtocol>();
    this.defs = new TreeMap<String, Binding>();
  }

  public CompilerUniverse getUniverse() {
    return universe;
  }

  public void declareInheritance(String sub, String shuper) {
    List<String> locals = rawInheritance.get(sub);
    if (locals == null) {
      locals = new ArrayList<String>();
      rawInheritance.put(sub, locals);
    }
    locals.add(shuper);
  }

  public void createDefinition(String name, Binding binding) {
    defs.put(name, binding);
  }

  public void createMethod(RMethod method) {
    methods.add(method);
  }

  public RProtocol createProtocol(Source origin, List<Annotation> annots, String id,
      String displayName) {
    RProtocol proto = new RProtocol(annots, id, displayName);
    protos.put(id, proto);
    defs.put(id, new Binding(
        Collections.<Annotation>emptyList(),
        new CodeBundle(
            origin.getName(),
            eConstant(proto),
            null,
            null)));
    return proto;
  }

  public RProtocol createImplicitProtocol(Source origin, String displayName) {
    int index = nextImplicitProtocolIndex++;
    String id = "implicit-" + index;
    return createProtocol(origin, Collections.<Annotation>emptyList(), id,
        displayName);
  }

  public Module getResult() {
    return new Module(defs, protos, methods, rawInheritance);
  }

}
