package org.neutrino.main;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.List;

import org.neutrino.oldton.PValue;
import org.neutrino.oldton.Oldton;
import org.neutrino.oldton.OldtonRegistry;
import org.neutrino.pib.Universe;

/**
 * Entry point to a simple utility dumping plankton files as text.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Dump {

  public static @Flags.Flag boolean raw = false;

  public static void main(String[] rawArgs) throws IOException {
    List<String> args = Flags.parseArguments(rawArgs, Dump.class);
    Oldton plankton = raw
        ? new Oldton(new OldtonRegistry())
        : Universe.getPlankton();
    for (String name : args) {
      File file = new File(name);
      PValue value = plankton.read(new FileInputStream(file));
      String str = Oldton.toString(value);
      System.out.println(str);
    }
  }

}
