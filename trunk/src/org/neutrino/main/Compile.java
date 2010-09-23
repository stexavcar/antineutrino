package org.neutrino.main;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import org.neutrino.compiler.CompilerModule;
import org.neutrino.pib.Universe;
import org.neutrino.pib.BinaryBuilder;
import org.neutrino.syntax.SyntaxError;

/**
 * Main entry-point to the java neutrino implementation.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Compile {

  public static void main(String[] args) throws IOException, SyntaxError {
    CompilerModule top = CompilerModule.createToplevel();
    for (String arg : args) {
      File path = new File(arg);
      top.includeFromPath(path);
    }
    Universe binary = compile(top);
    FileOutputStream out = new FileOutputStream("a.out");
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
