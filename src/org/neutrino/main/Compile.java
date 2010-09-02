package org.neutrino.main;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import org.neutrino.compiler.Module;
import org.neutrino.pib.Binary;
import org.neutrino.pib.BinaryBuilder;
import org.neutrino.syntax.SyntaxError;

/**
 * Main entry-point to the java neutrino implementation.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Compile {

  public static void main(String[] args) throws IOException, SyntaxError {
    Module top = Module.createToplevel();
    for (String arg : args) {
      File path = new File(arg);
      top.includeFromPath(path);
    }
    top.parseAll();
    BinaryBuilder builder = Binary.builder();
    top.writeToBinary(builder);
    Binary binary = builder.getResult();
    FileOutputStream out = new FileOutputStream("a.out");
    binary.write(out);
    out.close();
  }

}
