package org.neutrino.main;

import org.neutrino.pib.Universe;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PValue;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.RArray;
import org.neutrino.runtime.RLambda;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

public class Run {

  public static void main(String[] args) throws IOException {
    Interpreter inter = new Interpreter();
    String pibFile = args[0];
    String entryPointName = args[1];
    File file = new File(pibFile);
    PValue value = Universe.getPlankton().read(new FileInputStream(file));
    Universe binary = ((PSeed) value).grow(Universe.class);
    binary.initialize();
    RLambda entryPoint = binary.getEntryPoint(entryPointName);
    assert entryPoint != null : "No entry point found.";
    RValue entryPointValue = inter.interpret(entryPoint);
    RLambda entryPointLambda = (RLambda) entryPointValue;
    List<RValue> argList = new ArrayList<RValue>();
    for (int i = 2; i < args.length; i++)
      argList.add(new RString(args[i]));
    RArray argsObject = new RArray(argList);
    inter.interpret(entryPointLambda, argsObject);
  }

}
