package org.neutrino.pib;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

import org.neutrino.compiler.CompilerUniverse;
import org.neutrino.oldton.ISeedable;
import org.neutrino.oldton.Oldton;
import org.neutrino.oldton.OldtonRegistry;
import org.neutrino.oldton.PSeed;
import org.neutrino.oldton.PValue;
import org.neutrino.oldton.annotations.Growable;
import org.neutrino.oldton.annotations.SeedMember;
import org.neutrino.plankton.ClassIndex;
import org.neutrino.plankton.PlanktonEncoder;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.Lambda;
import org.neutrino.runtime.MethodLookupHelper;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RMethod;
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
@Growable(Universe.TAG)
public class Universe implements ISeedable {

  static final String TAG = "org::neutrino::pib::Universe";

  public @Store @SeedMember Map<String, Module> modules;
  private Universe parallelUniverse = null;
  private final MethodLookupHelper methodLookupHelper = new MethodLookupHelper(this);

  public Universe(Map<String, Module> modules) {
    this.modules = modules;
  }

  public Universe() { }

  public static BinaryBuilder builder(CompilerUniverse universe) {
    return new BinaryBuilder(universe);
  }

  public void setParallelUniverse(Universe universe) {
    this.parallelUniverse = universe;
  }

  public Universe getParallelUniverse() {
    return this.parallelUniverse;
  }

  public void initialize() {
    for (Module module : modules.values())
      module.initialize(this);
  }

  public Lambda getEntryPoint(String name) {
    for (Module module : modules.values()) {
      Lambda value = module.getEntryPoint(name);
      if (value != null)
        return value;
    }
    return (parallelUniverse == null)
        ? null
        : parallelUniverse.getEntryPoint(name);
  }

  public RValue getGlobal(String name, Interpreter inter) {
    for (Module module : modules.values()) {
      RValue value = module.getGlobal(name, inter);
      if (value != null)
        return value;
    }
    return (parallelUniverse == null)
        ? null
        : parallelUniverse.getGlobal(name, inter);
  }

  public RProtocol getProtocol(String name) {
    for (Module module : modules.values()) {
      RProtocol value = module.protos.get(name);
      if (value != null)
        return value;
    }
    return (parallelUniverse == null)
        ? null
        : parallelUniverse.getProtocol(name);
  }

  public RProtocol getProtocol(TypeId typeId) {
    for (Module module : modules.values()) {
      for (RProtocol proto : module.protos.values()) {
        if (proto.getInstanceTypeId() == typeId)
          return proto;
      }
    }
    return (parallelUniverse == null)
      ? null
      : parallelUniverse.getProtocol(typeId);
  }

  private final Map<TypeId, List<TypeId>> parentCache = new HashMap<TypeId, List<TypeId>>();

  public List<TypeId> getParents(TypeId id) {
    List<TypeId> result = parentCache.get(id);
    if (result == null) {
      result = new ArrayList<TypeId>();
      addParents(result, id);
      parentCache.put(id, result);
    }
    return result;
  }

  private void addParents(List<TypeId> out, TypeId id) {
    for (Module module : this.modules.values())
      module.addParents(out, id);
    if (parallelUniverse != null)
      parallelUniverse.addParents(out, id);
  }

  public Lambda lookupMethod(String name, int argc, Stack<RValue> stack) {
    return methodLookupHelper.lookupMethod(name, argc, stack);
  }

  public Lambda getLambda(RObject function, RValue... args) {
    return methodLookupHelper.lookupLambda(function, args);
  }

  public void writeOldton(OutputStream out) throws IOException {
    OLDTON.write(out, OLDTON.newSeed(this));
  }

  public void writePlankton(OutputStream out) throws IOException {
    PlanktonEncoder encoder = new PlanktonEncoder(REGISTRY.getClassIndex());
    encoder.write(this);
    encoder.flush();
    out.write(encoder.getBytes());
  }

  public Universe readOldton(InputStream in) throws IOException {
    PValue value = OLDTON.read(in);
    return ((PSeed) value).grow(Universe.class);
  }

  public static Oldton getOldton() {
    return OLDTON;
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
    return REGISTRY.getClassIndex();
  }

  private static final OldtonRegistry REGISTRY = new OldtonRegistry() {{
    register(Binding.class);
    register(Universe.class);
    register(Module.class);
    register(RProtocol.class);
    register(CodeBundle.class);
    register(RMethod.class);
    register(Parameter.class);
    register(Annotation.class);
    register(RString.class);
    register(Native.class);
    register(TypeId.class);
    register(RInteger.class);
  }};

  private static final Oldton OLDTON = new Oldton(REGISTRY);

}
