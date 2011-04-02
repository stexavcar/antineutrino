package org.neutrino.pib;

import static org.javatrino.ast.Expression.StaticFactory.eConstant;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.javatrino.ast.Method;
import org.neutrino.compiler.Source;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.Lambda;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;

public class Module {

  public @Store Map<String, Binding> defs;
  public @Store Map<String, RProtocol> protos;
  public @Store List<Method> methods;
  public @Store Universe universe;
  public @Store Map<RProtocol, List<RProtocol>> inheritance = new HashMap<RProtocol, List<RProtocol>>();

  private final Map<Object, RValue> globals = new HashMap<Object, RValue>();

  private Module(Universe universe, Map<String, Binding> defs, Map<String,
      RProtocol> protos, List<Method> methods, Map<RProtocol, List<RProtocol>> inheritance) {
    this.defs = defs;
    this.protos = protos;
    this.methods = methods;
    this.universe = universe;
    this.inheritance = inheritance;
  }

  public Module() { }

  public Universe getUniverse() {
    return universe;
  }

  public static Module newEmpty(Universe universe) {
    return new Module(universe, new HashMap<String, Binding>(),
        new HashMap<String, RProtocol>(), new ArrayList<Method>(),
        new HashMap<RProtocol, List<RProtocol>>());
  }

  public RProtocol getProtocol(String name) {
    RProtocol result = protos.get(name);
    return result == null ? universe.getProtocol(name) : result;
  }

  public Lambda getEntryPoint(String name) {
    for (Map.Entry<String, Binding> entry : defs.entrySet()) {
      Binding binding = entry.getValue();
      Annotation annot = binding.getAnnotation("entry_point");
      if (annot != null) {
        RValue value = annot.getArgument(0);
        if (((RString) value).getValue().equals(name))
          return new Lambda(this, binding.getCode());
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

  public RValue lookupGlobal(Object name) {
    RValue result = globals.get(name);
    if (result == null) {
      Binding binding = defs.get(name);
      if (binding == null)
        return null;
      result = Interpreter.run(binding.getCode());
      globals.put(name, result);
    }
    return result;
  }

  public RValue getGlobal(Object name) {
    return universe.getGlobal(name);
  }

  public void addParents(List<RProtocol> out, RProtocol id) {
    List<RProtocol> parents = inheritance.get(id);
    if (parents != null) {
      for (RProtocol parent : parents)
        out.add(parent);
    }
  }

  public void declareInheritance(RProtocol sub, RProtocol shuper) {
    List<RProtocol> locals = inheritance.get(sub);
    if (locals == null) {
      locals = new ArrayList<RProtocol>();
      inheritance.put(sub, locals);
    }
    locals.add(shuper);
  }

  public void createDefinition(String name, Binding binding) {
    defs.put(name, binding);
  }

  public void createMethod(Method method) {
    methods.add(method);
  }

  public RProtocol createProtocol(Source origin, List<Tree.Annotation> annots, String id,
      String displayName) {
    RProtocol proto = null;
    for (Tree.Annotation annot : annots) {
      if (annot.getTag().equals(Native.ANNOTATION)) {
        proto = RProtocol.getCanonical(((RString) annot.getArguments().get(0).getValue(this)).getValue());
      }
    }
    if (proto == null) {
      List<Annotation> newAnnots;
      if (annots.isEmpty()) {
        newAnnots = Collections.emptyList();
      } else {
        newAnnots = new ArrayList<Annotation>();
        for (Tree.Annotation annot : annots)
          newAnnots.add(new Annotation(annot.getTag(), null));
      }
      proto = new RProtocol(newAnnots, id, displayName);
    }
    protos.put(id, proto);
    defs.put(id, new Binding(
        Collections.<Annotation>emptyList(),
        new CodeBundle(
            this,
            origin.getName(),
            eConstant(proto),
            null,
            null)));
    return proto;
  }

}
