package org.neutrino.main;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.List;

import org.neutrino.pib.Universe;
import org.neutrino.plankton.PValue;
import org.neutrino.plankton.Plankton;
import org.neutrino.plankton.PlanktonRegistry;

/**
 * Entry point to a simple utility dumping plankton files as text.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Dump {

  public static @Flags.Flag boolean raw = false;

  public static void main(String[] rawArgs) throws IOException {
    List<String> args = Flags.parseArguments(rawArgs, Dump.class);
    Plankton plankton = raw
        ? new Plankton(new PlanktonRegistry())
        : Universe.getPlankton();
    for (String name : args) {
      File file = new File(name);
      PValue value = plankton.read(new FileInputStream(file));
      String str = Plankton.toString(value);
      System.out.println(str);
    }
  }

}
