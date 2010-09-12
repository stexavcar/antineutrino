package org.neutrino.pib;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Collection;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PValue;
import org.neutrino.plankton.Plankton;
import org.neutrino.plankton.PlanktonRegistry;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.runtime.RMethod;
import org.neutrino.runtime.RProtocol;

/**
 * A platform-independent binary.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
@Growable(Binary.TAG)
public class Binary implements ISeedable {

  private static final String TAG = "org::neutrino::pib::Binary";

  public @SeedMember Map<String, Module> modules;

  public Binary(Map<String, Module> modules) {
    this.modules = modules;
  }

  public Binary() { }

  public static BinaryBuilder builder() {
    return new BinaryBuilder();
  }

  public void write(OutputStream out) throws IOException {
    PLANKTON.write(out, PLANKTON.newSeed(this));
  }

  public Binary read(InputStream in) throws IOException {
    PValue value = PLANKTON.read(in);
    return ((PSeed) value).grow(Binary.class);
  }

  public static Plankton getPlankton() {
    return PLANKTON;
  }

  public Collection<Module> getModules() {
    return modules.values();
  }

  @Override
  public String toString() {
    return "a Binary " + modules;
  }

  public Binding getEntryPoint() {
    Binding entryPoint = null;
    for (Module module : getModules()) {
      for (Binding def : module.getDefinitions()) {
        if (def.getAnnotations().contains("entry_point")) {
          entryPoint = def;
        }
      }
    }
    return entryPoint;
  }

  private static final PlanktonRegistry REGISTRY = new PlanktonRegistry() {{
    register(Binding.class);
    register(Binary.class);
    register(Module.class);
    register(RProtocol.class);
    register(CodeBundle.class);
    register(RMethod.class);
    register(Parameter.class);
  }};

  private static final Plankton PLANKTON = new Plankton(REGISTRY);

}
