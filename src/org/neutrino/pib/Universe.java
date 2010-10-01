package org.neutrino.pib;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PValue;
import org.neutrino.plankton.Plankton;
import org.neutrino.plankton.PlanktonRegistry;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RLambda;
import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.syntax.Annotation;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Map;
import java.util.Set;

/**
 * A platform-independent binary.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
@Growable(Universe.TAG)
public class Universe implements ISeedable {

  private static final String TAG = "org::neutrino::pib::Universe";

  public @SeedMember Map<String, Module> modules;

  public Universe(Map<String, Module> modules) {
    this.modules = modules;
  }

  public Universe() { }

  public static BinaryBuilder builder() {
    return new BinaryBuilder();
  }

  public void initialize() {
    for (Module module : modules.values())
      module.initialize(this);
  }

  public RLambda getEntryPoint(String name) {
    for (Module module : modules.values()) {
      RLambda value = module.getEntryPoint(name);
      if (value != null)
        return value;
    }
    return null;
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
  }};

  private static final Plankton PLANKTON = new Plankton(REGISTRY);

}
