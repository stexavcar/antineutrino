package org.neutrino.compiler;

import java.io.File;
import java.io.IOException;
import java.util.Map;
import java.util.TreeMap;

import org.neutrino.pib.BinaryBuilder;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.syntax.SyntaxError;

/**
 * A single neutrino module.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Module {

  private final String name;
  private final Map<String, Module> modules = new TreeMap<String, Module>();
  private final Map<String, Source> sources = new TreeMap<String, Source>();

  private Module(String name) {
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
        else fullName = name + "." + shortName;
        Module child = new Module(fullName);
        child.includeFromPath(file);
        modules.put(shortName, child);
      } else {
        Source source = Source.create(file);
        if (source != null)
          sources.put(source.getName(), source);;
      }
    }
  }

  /**
   * Ensure that all source files have been fully parsed.  Any syntax
   * errors will be issued at this point.
   */
  public void parseAll() throws SyntaxError {
    for (Source source : sources.values())
      source.ensureParsed();
    for (Module child : modules.values())
      child.parseAll();
  }

  public void writeToBinary(BinaryBuilder builder) {
    if (!isToplevel()) {
      ModuleBuilder module = builder.createModule(name);
      for (Source source : sources.values())
        source.writeToBinary(module);
    }
    for (Module child : modules.values())
      child.writeToBinary(builder);
  }

  public static Module createToplevel() {
    return new Module(null);
  }

  @Override
  public String toString() {
    String start = (name == null) ? "toplevel" : ("module " + name);
    return start + " " + modules + " " + sources;
  }

}
