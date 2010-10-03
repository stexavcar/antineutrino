package org.neutrino.main;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import org.neutrino.compiler.CompilerModule;
import org.neutrino.pib.BinaryBuilder;
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

  public static void main(String[] rawArgs) throws IOException, SyntaxError {
    Flags.parseArguments(rawArgs, Compile.class);
    assert rootPath != null: "No --root-path specified.";
    CompilerModule top = CompilerModule.createToplevel();
    File path = new File(rootPath);
    top.includeFromPath(path);
    Universe binary = compile(top);
    FileOutputStream out = new FileOutputStream(outputPath);
    binary.write(out);
    out.close();
  }

  public static Universe compile(CompilerModule top) throws SyntaxError {
    top.parseAll();
    BinaryBuilder builder = Universe.builder();
    top.writeToBinary(builder);
    return builder.getResult();
  }

}
