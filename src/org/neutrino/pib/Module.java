package org.neutrino.pib;

import static org.javatrino.ast.Expression.StaticFactory.eConstant;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.compiler.Source;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.Lambda;
import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.runtime.TypeId;
import org.neutrino.syntax.Annotation;

public class Module {

  public @Store Map<String, Binding> defs;
  public @Store Map<String, RProtocol> protos;
  public @Store Map<String, List<String>> rawInheritance;
  public @Store List<RMethod> methods;
  public @Store Universe universe;

  private final Map<TypeId, List<TypeId>> inheritance = new HashMap<TypeId, List<TypeId>>();
  private final Map<Object, RValue> globals = new HashMap<Object, RValue>();

  public Module(Universe universe, Map<String, Binding> defs, Map<String,
      RProtocol> protos, List<RMethod> methods, Map<String, List<String>> inheritance) {
    this.defs = defs;
    this.protos = protos;
    this.methods = methods;
    this.rawInheritance = inheritance;
    this.universe = universe;
  }

  public Module() { }

  public Universe getUniverse() {
    return universe;
  }

  public static Module newEmpty(Universe universe) {
    return new Module(universe, new HashMap<String, Binding>(),
        new HashMap<String, RProtocol>(), new ArrayList<RMethod>(),
        new HashMap<String, List<String>>());
  }

  public void initialize() {
    for (RMethod method : methods)
      method.initialize(this);
    for (Map.Entry<String, List<String>> entry : rawInheritance.entrySet()) {
      String name = entry.getKey();
      RProtocol proto = getProtocol(name);
      assert proto != null : "Protocol " + name + " not found.";
      List<String> rawParents = entry.getValue();
      List<TypeId> parents = new ArrayList<TypeId>();
      for (String rawParent : rawParents) {
        RProtocol parentProto = getProtocol(rawParent);
        assert parentProto != null : "Protocol " + rawParent + " not found.";
        parents.add(parentProto.getInstanceTypeId());
      }
      inheritance.put(proto.getInstanceTypeId(), parents);
    }
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

  public RValue lookupGlobal(Object name, Interpreter inter) {
    RValue result = globals.get(name);
    if (result == null) {
      Binding binding = defs.get(name);
      if (binding == null)
        return null;
      result = inter.interpret(binding.getCode());
      globals.put(name, result);
    }
    return result;
  }

  public RValue getGlobal(Object name, Interpreter inter) {
    return universe.getGlobal(name, inter);
  }

  public void addParents(List<TypeId> out, TypeId id) {
    List<TypeId> parents = inheritance.get(id);
    if (parents != null) {
      for (TypeId parent : parents)
        out.add(parent);
    }
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
            this,
            origin.getName(),
            eConstant(proto),
            null,
            null)));
    return proto;
  }

}
