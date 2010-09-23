package org.neutrino.main;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
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

  public static void main(String[] argArray) throws IOException {
    Plankton plankton = Universe.getPlankton();
    List<String> args = new ArrayList<String>(Arrays.asList(argArray));
    for (String arg : args) {
      if ("-n".equals(arg)) {
        plankton = new Plankton(new PlanktonRegistry());
        args.remove(arg);
      }
    }
    for (String name : args) {
      File file = new File(name);
      PValue value = plankton.read(new FileInputStream(file));
      String str = Plankton.toString(value);
      System.out.println(str);
    }
  }

}
