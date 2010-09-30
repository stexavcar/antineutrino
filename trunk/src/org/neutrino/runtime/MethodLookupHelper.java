package org.neutrino.runtime;

import org.neutrino.pib.Module;
import org.neutrino.pib.Parameter;

import java.util.ArrayList;
import java.util.List;

public class MethodLookupHelper {

  private final Module module;
  private final List<RMethod> matches = new ArrayList<RMethod>();

  public MethodLookupHelper(Module module) {
    this.module = module;
  }

  public RLambda lookupMethod(String name, int argc, Frame frame) {
    if ("()".equals(name))
      return (RLambda) frame.stack.get(frame.stack.size() - argc);
    matches.clear();
    loop: for (RMethod method : module.methods) {
      if (method.getName().equals(name)) {
        List<Parameter> params = method.getParameters();
        if (params.size() != argc)
          continue loop;
        for (int i = 0; i < argc; i++) {
          TypeId paramType = params.get(i).getTypeId();
          if (paramType == null)
            continue;
          RValue arg = frame.getArgument(argc, i);
          TypeId argType = arg.getTypeId();
          if (argType != paramType)
            continue loop;
        }
        matches.add(method);
      }
    }
    RMethod result = null;
    if (matches.size() == 0) {
      return null;
    } else if (matches.size() == 1) {
      result = matches.get(0);
    } else {
      for (RMethod method : matches) {
        result = method;
      }
    }
    return new RLambda(module, result.getCode());
  }

}
