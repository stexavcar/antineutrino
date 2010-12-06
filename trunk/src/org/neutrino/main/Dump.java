package org.neutrino.main;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;

import org.neutrino.pib.Universe;
import org.neutrino.plankton.ClassIndex;
import org.neutrino.plankton.PlanktonDecoder;

/**
 * Entry point to a simple utility dumping plankton files as text.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Dump {

  public static @Flags.Flag boolean raw = false;

  public static void main(String[] rawArgs) throws IOException {
    List<String> args = Flags.parseArguments(rawArgs, Dump.class);
    for (String name : args) {
      File file = new File(name);
      InputStream in = new FileInputStream(file);
      PlanktonDecoder plankton =
        new PlanktonDecoder(raw ? new ClassIndex() : Universe.getClassIndex(), in);
      Object value = plankton.read();
      System.out.println(value);
    }
  }

}
