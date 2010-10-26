package org.neutrino.main;

import org.neutrino.compiler.CompilerModule;
import org.neutrino.compiler.CompilerUniverse;
import org.neutrino.pib.BinaryBuilder;
import org.neutrino.pib.Universe;
import org.neutrino.syntax.SyntaxError;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

/**
 * Main entry-point to the java neutrino implementation.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Compile {

  public static @Flags.Flag("root-path") String rootPath;
  public static @Flags.Flag("outfile") String outputPath = "a.pib";

  public static void main(String[] rawArgs) throws IOException, SyntaxError {
    Flags.parseArguments(rawArgs, Compile.class);
    assert rootPath != null: "No --root-path specified.";
    CompilerModule top = CompilerModule.createToplevel();
    CompilerUniverse universe = new CompilerUniverse(top);
    File path = new File(rootPath);
    top.includeFromPath(path);
    Universe binary = compile(universe);
    if (binary == null)
      System.exit(1);
    FileOutputStream out = new FileOutputStream(outputPath);
    binary.write(out);
    out.close();
  }

  public static Universe compile(CompilerUniverse top) {
    try {
      top.parseAll();
    } catch (SyntaxError se) {
      System.out.println(se.getFileName() + ":" + se.getLineNumber() +
          ": " + se.getMessage());
      for (String line : se.getSourceUnderline())
        System.out.println(line);
      return null;
    }
    BinaryBuilder builder = Universe.builder(top);
    top.writeToBinary(builder);
    return builder.getResult();
  }

}
