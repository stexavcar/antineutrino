package org.neutrino.compiler;

import org.neutrino.pib.Universe;
import org.neutrino.runtime.RModule;
import org.neutrino.runtime.RValue;
import org.neutrino.syntax.SyntaxError;


public class CompilerUniverse {

  private final CompilerModule root;

  public CompilerUniverse(CompilerModule root) {
    this.root = root;
  }

  public void parseAll() throws SyntaxError {
    root.parseAll();
  }

  public void preCompileModules(Universe universe) {
    root.buildSkeleton(universe);
  }

  private void loadModuleManifests(Universe universe) {
    // First load primordial modules.
    for (CompilerModule module : root.getModules()) {
      if (module.isPrimordial())
        module.ensureWritten(universe);
    }
    // Load all manifests.
    for (CompilerModule module : root.getModules()) {
      if (!module.isPrimordial())
        module.writeManifest(universe);
    }
    // Bind handlers for all non-primordial modules.
    for (CompilerModule module : root.getModules()) {
      if (!module.isPrimordial())
        module.loadHandler(universe);
    }
    // Then load the rest
    for (CompilerModule module : root.getModules()) {
        module.ensureWritten(universe);
    }
    // Configure modules.
    for (CompilerModule module : root.getModules()) {
      module.runCustomConfigurator(universe);
    }
  }

  public CompilerModule getModule(RValue name) {
    for (CompilerModule module : root.getModules()) {
      RModule obj = module.getModuleObject();
      if (obj.getName().equals(name))
        return module;
    }
    return null;
  }

  public void writeTo(Universe universe) {
    loadModuleManifests(universe);
  }

}
