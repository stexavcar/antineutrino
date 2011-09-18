package org.neutrino.pib;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.IdentityHashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

import org.javatrino.ast.Expression;
import org.neutrino.plankton.ClassIndex;
import org.neutrino.plankton.IBuiltinObjectIndex;
import org.neutrino.plankton.PlanktonEncoder;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.Lambda;
import org.neutrino.runtime.MethodLookupHelper;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RBoolean;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RFunction;
import org.neutrino.runtime.RImpl;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RModule;
import org.neutrino.runtime.RNull;
import org.neutrino.runtime.RObject;
import org.neutrino.runtime.RObjectArray;
import org.neutrino.runtime.RPlatform;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.runtime.lookup.CallInfo;
import org.neutrino.syntax.Annotation;

/**
 * A platform-independent binary.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Universe {

  private final RPlatform platform = new RPlatform(this);
  public @Store Map<RValue, Module> modules;
  private final MethodLookupHelper methodLookupHelper = new MethodLookupHelper(this);

  public Universe(Map<RValue, Module> modules) {
    this.modules = modules;
  }

  public Universe() { }

  public static Universe newEmpty() {
    return new Universe(new HashMap<RValue, Module>());
  }

  public void retainModules(List<String> values) {
    if (values.isEmpty())
      return;
    Set<RString> strObjs = new HashSet<RString>();
    for (String str : values)
      strObjs.add(RString.of(str));
    modules.keySet().retainAll(strObjs);
  }

  public RPlatform getPlatform() {
    return platform;
  }

  public Module createModule(RValue name, RModule wrapper) {
    Module result = Module.newEmpty(this, wrapper);
    modules.put(name, result);
    return result;
  }

  public RValue getEntryPoint(RValue name) {
    for (Module module : modules.values()) {
      RValue value = module.getEntryPoint(name);
      if (value != null)
        return value;
    }
    return null;
  }

  private final Map<RProtocol, RProtocol[]> parentCache = new HashMap<RProtocol, RProtocol[]>();

  public RProtocol[] getParents(RProtocol id) {
    RProtocol[] result = parentCache.get(id);
    if (result == null) {
      List<RProtocol> elms = new ArrayList<RProtocol>();
      addParents(elms, id);
      result = elms.toArray(new RProtocol[elms.size()]);
      parentCache.put(id, result);
    }
    return result;
  }

  private void addParents(List<RProtocol> out, RProtocol id) {
    for (Module module : this.modules.values())
      module.addParents(out, id);
  }

  public Lambda lookupMethod(CallInfo info, Stack<RValue> stack) {
    return methodLookupHelper.lookupMethod(info, stack);
  }

  public Lambda getLambda(String name, RValue... args) {
    return methodLookupHelper.lookupLambda(name, args);
  }

  public void writePlankton(OutputStream out) throws IOException {
    PlanktonEncoder encoder = new PlanktonEncoder(getClassIndex(), getBuiltinIndex());
    encoder.write(this);
    encoder.flush();
    out.write(encoder.getBytes());
  }

  public void evaluateStatics() {
    for (Module module : modules.values())
      module.evaluateStatics();
  }

  public Set<Map.Entry<RValue, Module>> getModules() {
    return modules.entrySet();
  }

  public Module getModule(RValue name) {
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
    add(RInteger.class);
    add(RFieldKey.class);
    add(RObject.class);
    add(RFunction.class);
    add(RValue.State.class);
    add(RImpl.class);
    add(RObjectArray.class);
    add(RBoolean.class);
    add(RNull.class);
    add(RModule.class);
    Expression.register(this);
  }};

  public static final IBuiltinObjectIndex getBuiltinIndex() {
    return BUILT_IN_INDEX;
  }

  private static final Map<Object, RValue> BUILTINS_BY_KEY = new HashMap<Object, RValue>();
  private static final Map<RValue, Object> KEYS_BY_BUILTIN = new IdentityHashMap<RValue, Object>();

  private static void addBuiltin(Object key, RValue value) {
    BUILTINS_BY_KEY.put(key, value);
    KEYS_BY_BUILTIN.put(value, key);
  }

  private static final IBuiltinObjectIndex BUILT_IN_INDEX = new IBuiltinObjectIndex() {

    @Override
    public RValue getValue(Object key) {
      return BUILTINS_BY_KEY.get(key);
    }

    @Override
    public Object getKey(RValue value) {
      return KEYS_BY_BUILTIN.get(value);
    }
  };

  private static final List<String> PROTOCOLS = Arrays.asList(
      "Protocol", "FieldKey", "String", "Integer", "ObjectArray", "ByteArray",
      "True", "False", "continuation", "file", "null", "ref", "Lambda",
      "Function", "PrimitiveMap", "Module", "Platform");

  static {
    for (String name : PROTOCOLS) {
      RValue key = RString.of(name);
      addBuiltin(name, new RProtocol(Annotation.NONE, key, key));
    }
  }

  public RValue call(String name, RValue... args) {
    Lambda lambda = getLambda(name, args);
    assert lambda != null;
    return Interpreter.run(lambda, args);
  }

}
