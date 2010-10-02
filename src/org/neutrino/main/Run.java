package org.neutrino.main;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.neutrino.pib.Universe;
import org.neutrino.plankton.PSeed;
import org.neutrino.plankton.PValue;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.RLambda;
import org.neutrino.runtime.RObjectArray;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;

public class Run {

  public static @Flags.Flag("pib-path") String pibPath;
  public static @Flags.Flag("entry-point") String entryPoint = "main";

  private static RObjectArray buildStringList(List<String> list) {
    List<RValue> result = new ArrayList<RValue>();
    for (int i = 0; i < list.size(); i++)
      result.add(new RString(list.get(i)));
    RObjectArray argsObject = new RObjectArray(result);
    return argsObject;
  }

  public static void main(String[] rawArgs) throws IOException {
    List<String> args = Flags.parseArguments(rawArgs, Run.class);
    Interpreter inter = new Interpreter();
    File file = new File(pibPath);
    PValue value = Universe.getPlankton().read(new FileInputStream(file));
    Universe binary = ((PSeed) value).grow(Universe.class);
    binary.initialize();
    RLambda entryPointBinding = binary.getEntryPoint(entryPoint);
    assert entryPointBinding != null : "No entry point found.";
    RValue entryPointValue = inter.interpret(entryPointBinding);
    RLambda entryPointLambda = (RLambda) entryPointValue;
    RObjectArray argsObject = buildStringList(args);
    inter.interpret(entryPointLambda, argsObject);
  }

}
