package org.neutrino.main;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import org.neutrino.pib.Binary;
import org.neutrino.pib.Binding;
import org.neutrino.pib.Module;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PValue;
import org.neutrino.runtime.Interpreter;

public class Run {

  public static void main(String[] args) throws IOException {
    Interpreter inter = new Interpreter();
    for (String name : args) {
      File file = new File(name);
      PValue value = Binary.getPlankton().read(new FileInputStream(file));
      Binary binary = ((PSeed) value).grow(Binary.class);
      Binding entryPoint = null;
      for (Module module : binary.getModules()) {
        for (Binding def : module.getDefinitions()) {
          if (def.getAnnotations().contains("entry_point")) {
            entryPoint = def;
          }
        }
      }
      assert entryPoint != null : "No entry point found.";
      Object result = inter.interpret(entryPoint.getCode());
      System.out.println(result);
    }
  }

}
