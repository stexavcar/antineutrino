package org.neutrino.compiler;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.neutrino.pib.Module;
import org.neutrino.pib.Universe;
import org.neutrino.runtime.RModule;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.syntax.SyntaxError;
import org.neutrino.syntax.Tree.Declaration;

/**
 * A single neutrino module.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class CompilerModule {

  private final RValue name;
  private final RModule module;
  private Source manifest;
  private final Map<RValue, CompilerModule> modules = new TreeMap<RValue, CompilerModule>();
  private final Map<String, Source> sources = new TreeMap<String, Source>();
  private boolean hasBeenWritten = false;
  private RValue handler;

  private CompilerModule(RValue name) {
    this.name = name;
    this.module = new RModule(name);
  }

  private boolean isToplevel() {
    return name == null;
  }

  public RModule getModuleObject() {
    return module;
  }

  /**
   * Load the contents of the given directory, recursively, into this
   * module.
   */
  public void includeFromPath(File path) throws IOException {
    assert path.exists() && path.isDirectory();
    for (File file : path.listFiles()) {
      if (file.isDirectory()) {
        RValue shortName = RString.of(file.getName());
        RValue fullName;
        if (name == null) fullName = shortName;
        else fullName = RString.of(name.toExternalString() + "::" + shortName.toExternalString());
        CompilerModule child = ensureModule(shortName, fullName);
        child.includeFromPath(file);
      } else {
        Source source = Source.createSource(file);
        if (source == null) {
          Source manifest = Source.createModule(file);
          if (manifest != null) {
            this.manifest = manifest;
          }
        } else {
          sources.put(source.getName(), source);
        }
      }
    }
  }

  public CompilerModule ensureModule(RValue shortName, RValue fullName) {
    if (!modules.containsKey(shortName))
      modules.put(shortName, new CompilerModule(fullName));
    return modules.get(shortName);
  }

  public void includeSource(String name, String str) {
    sources.put(name, Source.create(name, str));
  }

  /**
   * Ensure that all source files have been fully parsed.  Any syntax
   * errors will be issued at this point.
   */
  public void parseAll() throws SyntaxError {
    if (manifest != null)
      manifest.ensureParsed();
    for (Source source : sources.values())
      source.ensureParsed();
    for (CompilerModule child : modules.values())
      child.parseAll();
  }

  public boolean isPrimordial() {
    if (manifest == null)
      return false;
    return manifest.findAnnotated("primordial") != null;
  }

  public void buildSkeleton(Universe universe) {
    if (!isToplevel()) {
      Module module = universe.createModule(name, this.module);
      for (Source source : sources.values())
        source.buildSkeleton(module);
      if (manifest != null)
        manifest.buildSkeleton(module);
    }
    for (CompilerModule child : modules.values())
      child.buildSkeleton(universe);
  }

  public void writeManifest(Universe universe) {
    if (manifest == null)
      return;
    manifest.writeTo(universe.getModule(name));
  }

  public void loadHandler(Universe universe) {
    if (manifest == null)
      return;
    Declaration decl = manifest.findAnnotated("module");
    if (decl == null)
      return;
    RValue proto = decl.getMaterialized();
    if (proto == null)
      return;
    this.handler = universe.call("new", proto);
  }

  public void runCustomConfigurator(Universe universe) {
    if (this.handler == null)
      return;
    universe.call("configure", handler, module, universe.getPlatform());
  }

  public void ensureWritten(Universe universe) {
    if (!isToplevel() && !hasBeenWritten) {
      hasBeenWritten = true;
      Module module = universe.getModule(name);
      for (Source source : sources.values())
        source.writeTo(module);
    }
  }

  public static CompilerModule createToplevel() {
    return new CompilerModule(null);
  }

  @Override
  public String toString() {
    String start = (name == null) ? "toplevel" : ("module " + name);
    return start + " " + modules + " " + sources;
  }

  public List<CompilerModule> getModules() {
    List<CompilerModule> result = new ArrayList<CompilerModule>();
    addModules(result);
    return result;
  }

  private void addModules(List<CompilerModule> out) {
    for (CompilerModule child : modules.values()) {
      out.add(child);
      child.addModules(out);
    }
  }

}
