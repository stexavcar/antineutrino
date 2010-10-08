package org.neutrino.pib;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.Lambda;
import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.runtime.TypeId;
import org.neutrino.syntax.Annotation;

@Growable(Module.TAG)
public class Module implements ISeedable {

  public static final String TAG = "org::neutrino::pib::Module";

  public @SeedMember Map<String, Binding> defs;
  public @SeedMember Map<String, RProtocol> protos;
  public @SeedMember Map<String, List<String>> rawInheritance;
  public @SeedMember List<RMethod> methods;

  private final Map<TypeId, List<TypeId>> inheritance = new HashMap<TypeId, List<TypeId>>();
  private final Map<String, RValue> globals = new HashMap<String, RValue>();
  private Universe universe;

  public Module(Map<String, Binding> defs, Map<String, RProtocol> protos,
      List<RMethod> methods, Map<String, List<String>> inheritance) {
    this.defs = defs;
    this.protos = protos;
    this.methods = methods;
    this.rawInheritance = inheritance;
  }

  public Module() { }

  public Universe getUniverse() {
    return universe;
  }

  public void initialize(Universe universe) {
    this.universe = universe;
    for (RProtocol proto : protos.values())
      proto.initialize();
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

  public RValue getGlobal(String name, Interpreter inter) {
    RValue result = globals.get(name);
    if (result == null) {
      Binding binding = defs.get(name);
      if (binding == null)
        return null;
      result = inter.interpret(this, binding.getCode());
      globals.put(name, result);
    }
    return result;
  }

  public void addParents(List<TypeId> out, TypeId id) {
    List<TypeId> parents = inheritance.get(id);
    if (parents != null) {
      for (TypeId parent : parents)
        out.add(parent);
    }
  }

}