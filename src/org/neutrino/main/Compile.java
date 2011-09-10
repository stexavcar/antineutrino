package org.neutrino.main;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.neutrino.compiler.CompilerModule;
import org.neutrino.compiler.CompilerUniverse;
import org.neutrino.pib.Universe;
import org.neutrino.syntax.SyntaxError;

/**
 * Main entry-point to the java neutrino implementation.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Compile {

  public static @Flags.Flag("root-path") String rootPath;
  public static @Flags.Flag("outfile") String outputPath = "a.pib";
  public static @Flags.Flag("module") List<String> modules = new ArrayList<String>();

  public static void main(String[] rawArgs) throws IOException {
    Flags.parseArguments(rawArgs, Compile.class);
    assert rootPath != null: "No --root-path specified.";
    CompilerModule top = CompilerModule.createToplevel();
    CompilerUniverse universe = new CompilerUniverse(top);
    File path = new File(rootPath);
    top.includeFromPath(path);
    Universe binary = compile(universe);
    if (binary == null)
      System.exit(1);
    binary.retainModules(modules);
    FileOutputStream out = new FileOutputStream(outputPath);
    binary.writePlankton(out);
    out.close();
  }

  public static Universe compile(CompilerUniverse top) {
    Universe universe = Universe.newEmpty();
    try {
      top.parseAll();
    } catch (SyntaxError se) {
      System.out.println(se.getFileName() + ":" + se.getLineNumber() +
          ": " + se.getMessage());
      for (String line : se.getSourceUnderline())
        System.out.println(line);
      return null;
    }
    top.preCompileModules();
    top.writeTo(universe);
    universe.evaluateStatics();
    return universe;
  }

}
