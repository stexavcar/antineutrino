package org.neutrino.main;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.neutrino.pib.Universe;
import org.neutrino.plankton.PlanktonDecoder;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.Lambda;
import org.neutrino.runtime.RObject;
import org.neutrino.runtime.RPrimitiveArray;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;

public class Run {

  public static @Flags.Flag("pib-path") String pibPath;
  public static @Flags.Flag("entry-point") String entryPoint = "main";

  private static RPrimitiveArray buildStringList(List<String> list) {
    List<RValue> result = new ArrayList<RValue>();
    for (int i = 0; i < list.size(); i++)
      result.add(new RString(list.get(i)));
    RPrimitiveArray argsObject = new RPrimitiveArray(result);
    return argsObject;
  }

  public static void main(String[] rawArgs) throws IOException {
    List<String> args = Flags.parseArguments(rawArgs, Run.class);
    File file = new File(pibPath);
    PlanktonDecoder decoder = new PlanktonDecoder(Universe.getClassIndex(),
        Universe.getBuiltinIndex(), new FileInputStream(file));
    Universe universe = (Universe) decoder.read();
    Lambda entryPointBinding = universe.getEntryPoint(entryPoint);
    assert entryPointBinding != null : "No entry point found.";
    RValue entryPointValue = Interpreter.run(entryPointBinding);
    RPrimitiveArray argsObject = buildStringList(args);
    Lambda entryPointLambda = universe.getLambda((RObject) entryPointValue,
        argsObject);
    if (entryPointLambda == null)
      entryPointLambda = universe.getLambda((RObject) entryPointValue);
    Interpreter.run(entryPointLambda, argsObject);
  }

}
