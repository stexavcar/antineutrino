package org.neutrino.main;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import org.neutrino.pib.Universe;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PValue;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.RLambda;
import org.neutrino.runtime.RValue;

public class Run {

  public static void main(String[] args) throws IOException {
    Interpreter inter = new Interpreter();
    for (String name : args) {
      File file = new File(name);
      PValue value = Universe.getPlankton().read(new FileInputStream(file));
      Universe binary = ((PSeed) value).grow(Universe.class);
      binary.initialize();
      RLambda entryPoint = binary.getEntryPoint();
      assert entryPoint != null : "No entry point found.";
      RValue entryPointValue = inter.interpret(entryPoint);
      RLambda entryPointLambda = (RLambda) entryPointValue;
      RValue result = inter.interpret(entryPointLambda);
      System.out.println(result);
    }
  }

}
