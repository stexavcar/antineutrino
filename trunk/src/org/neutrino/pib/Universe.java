package org.neutrino.pib;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

import org.javatrino.ast.Expression;
import org.neutrino.plankton.ClassIndex;
import org.neutrino.plankton.PlanktonEncoder;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.Lambda;
import org.neutrino.runtime.MethodLookupHelper;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RObject;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.runtime.TypeId;
import org.neutrino.syntax.Annotation;

/**
 * A platform-independent binary.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Universe {

  public @Store Map<String, Module> modules;
  private final MethodLookupHelper methodLookupHelper = new MethodLookupHelper(this);

  public Universe(Map<String, Module> modules) {
    this.modules = modules;
  }

  public Universe() { }

  public static Universe newEmpty() {
    return new Universe(new HashMap<String, Module>());
  }

  public void initialize() {
    for (Module module : modules.values())
      module.initialize();
  }

  public Module createModule(String name) {
    Module result = Module.newEmpty(this);
    modules.put(name, result);
    return result;
  }

  public Lambda getEntryPoint(String name) {
    for (Module module : modules.values()) {
      Lambda value = module.getEntryPoint(name);
      if (value != null)
        return value;
    }
    return null;
  }

  public RValue getGlobal(Object name, Interpreter inter) {
    for (Module module : modules.values()) {
      RValue value = module.lookupGlobal(name, inter);
      if (value != null)
        return value;
    }
    return null;
  }

  public RProtocol getProtocol(String name) {
    for (Module module : modules.values()) {
      RProtocol value = module.protos.get(name);
      if (value != null)
        return value;
    }
    return null;
  }

  public RProtocol getProtocol(TypeId typeId) {
    for (Module module : modules.values()) {
      for (RProtocol proto : module.protos.values()) {
        if (proto.getInstanceTypeId() == typeId)
          return proto;
      }
    }
    return null;
  }

  private final Map<TypeId, TypeId[]> parentCache = new HashMap<TypeId, TypeId[]>();

  public TypeId[] getParents(TypeId id) {
    TypeId[] result = parentCache.get(id);
    if (result == null) {
      List<TypeId> elms = new ArrayList<TypeId>();
      addParents(elms, id);
      result = elms.toArray(new TypeId[elms.size()]);
      parentCache.put(id, result);
    }
    return result;
  }

  private void addParents(List<TypeId> out, TypeId id) {
    for (Module module : this.modules.values())
      module.addParents(out, id);
  }

  public Lambda lookupMethod(String name, int argc, Stack<RValue> stack) {
    return methodLookupHelper.lookupMethod(name, argc, stack);
  }

  public Lambda getLambda(RObject function, RValue... args) {
    return methodLookupHelper.lookupLambda(function, args);
  }

  public void writePlankton(OutputStream out) throws IOException {
    PlanktonEncoder encoder = new PlanktonEncoder(getClassIndex());
    encoder.write(this);
    encoder.flush();
    out.write(encoder.getBytes());
  }

  public Set<Map.Entry<String, Module>> getModules() {
    return modules.entrySet();
  }

  public Module getModule(String name) {
    return modules.get(name);
  }

  @Override
  public String toString() {
    return "a Universe " + modules;
  }

  public static ClassIndex getClassIndex() {
    return REGISTRY;
  }

  private static final ClassIndex REGISTRY = new ClassIndex() {{
	add(Binding.class);
    add(Universe.class);
    add(Module.class);
    add(RProtocol.class);
    add(CodeBundle.class);
    add(Parameter.class);
    add(Annotation.class);
    add(RString.class);
    add(Native.class);
    add(TypeId.class);
    add(RInteger.class);
    add(RFieldKey.class);
    Expression.register(this);
  }};

}
