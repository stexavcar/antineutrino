package org.neutrino.pib;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PValue;
import org.neutrino.plankton.Plankton;
import org.neutrino.plankton.PlanktonRegistry;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.runtime.Frame;
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

  public @SeedMember Map<String, Module> modules;
  private Universe parallelUniverse = null;
  private final MethodLookupHelper methodLookupHelper = new MethodLookupHelper(this);

  public Universe(Map<String, Module> modules) {
    this.modules = modules;
  }

  public Universe() { }

  public static BinaryBuilder builder() {
    return new BinaryBuilder();
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


  public List<TypeId> getParents(TypeId id) {
    List<TypeId> parents = new ArrayList<TypeId>();
    addParents(parents, id);
    return parents;
  }

  private void addParents(List<TypeId> out, TypeId id) {
    for (Module module : this.modules.values())
      module.addParents(out, id);
    if (parallelUniverse != null)
      parallelUniverse.addParents(out, id);
  }

  public Lambda lookupMethod(String name, int argc, Frame frame) {
    return methodLookupHelper.lookupMethod(name, argc, frame);
  }

  public Lambda getLambda(RObject function, RValue... args) {
    return methodLookupHelper.lookupLambda(function, args);
  }

  public void write(OutputStream out) throws IOException {
    PLANKTON.write(out, PLANKTON.newSeed(this));
  }

  public Universe read(InputStream in) throws IOException {
    PValue value = PLANKTON.read(in);
    return ((PSeed) value).grow(Universe.class);
  }

  public static Plankton getPlankton() {
    return PLANKTON;
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

  private static final PlanktonRegistry REGISTRY = new PlanktonRegistry() {{
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

  private static final Plankton PLANKTON = new Plankton(REGISTRY);

}
