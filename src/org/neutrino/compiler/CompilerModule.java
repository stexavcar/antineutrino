package org.neutrino.compiler;

import org.neutrino.main.Flags;
import org.neutrino.pib.BinaryBuilder;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.syntax.SyntaxError;
import org.neutrino.syntax.Tree;

import java.io.File;
import java.io.IOException;
import java.util.Map;
import java.util.TreeMap;

/**
 * A single neutrino module.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class CompilerModule {

  public static @Flags.Flag("module") String module = null;

  private final String name;
  private final Map<String, CompilerModule> modules = new TreeMap<String, CompilerModule>();
  private final Map<String, Source> sources = new TreeMap<String, Source>();

  private CompilerModule(String name) {
    this.name = name;
  }

  private boolean isToplevel() {
    return name == null;
  }

  /**
   * Load the contents of the given directory, recursively, into this
   * module.
   */
  public void includeFromPath(File path) throws IOException {
    assert path.exists() && path.isDirectory();
    for (File file : path.listFiles()) {
      if (file.isDirectory()) {
        String shortName = file.getName();
        String fullName;
        if (name == null) fullName = shortName;
        else fullName = name + "::" + shortName;
        CompilerModule child = ensureModule(shortName, fullName);
        child.includeFromPath(file);
      } else if (module == null || module.equals(name)) {
        Source source = Source.create(file);
        if (source != null)
          sources.put(source.getName(), source);
      }
    }
  }

  public Tree.Declaration findDeclaration(String name) {
    for (Source source : sources.values()) {
      Tree.Declaration result = source.findDeclaration(name);
      if (result != null)
        return result;
    }
    for (CompilerModule module : modules.values()) {
      Tree.Declaration result = module.findDeclaration(name);
      if (result != null)
        return result;
    }
    return null;
  }

  public Tree.Method findMethod(String holder, String name) {
    for (Source source : sources.values()) {
      Tree.Method result = source.findMethod(holder, name);
      if (result != null)
        return result;
    }
    for (CompilerModule module : modules.values()) {
      Tree.Method result = module.findMethod(holder, name);
      if (result != null)
        return result;
    }
    return null;
  }

  public CompilerModule ensureModule(String shortName, String fullName) {
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
    for (Source source : sources.values())
      source.ensureParsed();
    for (CompilerModule child : modules.values())
      child.parseAll();
  }

  public void writeToBinary(BinaryBuilder builder) {
    if (!isToplevel()) {
      ModuleBuilder module = builder.createModule(name);
      for (Source source : sources.values())
        source.writeToBinary(module);
    }
    for (CompilerModule child : modules.values())
      child.writeToBinary(builder);
  }

  public static CompilerModule createToplevel() {
    return new CompilerModule(null);
  }

  @Override
  public String toString() {
    String start = (name == null) ? "toplevel" : ("module " + name);
    return start + " " + modules + " " + sources;
  }

}
