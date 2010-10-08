package org.neutrino.runtime;

import java.util.List;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Parameter;
import org.neutrino.pib.Universe;

public class MethodLookupHelper {

  private static final int kUnrelated = Integer.MAX_VALUE;
  private static final int kUnspecified = Integer.MAX_VALUE / 2;
  private final Universe universe;
  private int[] bestScore;
  private int[] candidateScore;
  private RMethod bestMethod;

  public MethodLookupHelper(Universe universe) {
    this.universe = universe;
  }

  public Lambda lookupMethod(String name, int argc, Frame frame) {
    RMethod result = findPerfectMatch(name, argc, frame, universe);
    if (result == null)
      result = findApproximateMatch(name, argc, frame, universe);
    if (result != null)
      return new Lambda(result.getOrigin(), result.getCode());
    else
      return null;
  }

  public Lambda lookupLambda(RObject holder, RValue... args) {
    Frame fakeFrame = new Frame(null, null, new CodeBundle(), null);
    fakeFrame.stack.push(holder);
    for (RValue arg : args)
      fakeFrame.stack.push(arg);
    return lookupMethod("()", 1 + args.length, fakeFrame);
  }

  private RMethod findPerfectMatch(String name, int argc, Frame frame,
      Universe universe) {
    for (Module module : universe.modules.values()) {
      RMethod result = findPerfectMatch(module, name, argc, frame);
      if (result != null)
        return result;
    }
    Universe parallel = universe.getParallelUniverse();
    if (parallel != null)
      return findPerfectMatch(name, argc, frame, parallel);
    return null;
  }

  private RMethod findApproximateMatch(String name, int argc, Frame frame,
      Universe universe) {
    bestMethod = null;
    bestScore = new int[argc];
    candidateScore = new int[argc];
    for (int i = 0; i < argc; i++)
      bestScore[i] = Integer.MAX_VALUE;
    for (Module module : universe.modules.values())
      findApproximateMatch(module, name, argc, frame);
    Universe parallel = universe.getParallelUniverse();
    if (parallel != null)
      findApproximateMatch(name, argc, frame, parallel);
    return bestMethod;
  }


  private RMethod findPerfectMatch(Module module, String name, int argc, Frame frame) {
    loop: for (RMethod method : module.methods) {
      if (method.getName().equals(name)) {
        List<Parameter> params = method.getParameters();
        if (params.size() != argc)
          continue loop;
        for (int i = 0; i < argc; i++) {
          TypeId paramType = params.get(i).getTypeId();
          if (paramType == null)
            paramType = TypeId.get("Object");
          RValue arg = frame.getArgument(argc, i);
          TypeId argType = arg.getTypeId();
          if (argType != paramType)
            continue loop;
        }
        return method;
      }
    }
    return null;
  }

  private RMethod findApproximateMatch(Module module, String name, int argc, Frame frame) {
    loop: for (RMethod method : module.methods) {
      if (!method.getName().equals(name))
        continue loop;
      List<Parameter> params = method.getParameters();
      if (params.size() != argc)
        continue loop;
      boolean isBetter = false;
      for (int i = 0; i < argc; i++) {
        TypeId paramType = params.get(i).getTypeId();
        int score;
        if (paramType == null) {
          score = kUnspecified;
        } else {
          RValue arg = frame.getArgument(argc, i);
          TypeId argType = arg.getTypeId();
          score = getScore(paramType, argType);
          if (score == kUnrelated)
            continue loop;
        }
        int prevScore = bestScore[i];
        if (prevScore > score) {
          isBetter = true;
        } else if (prevScore < score) {
          continue loop;
        }
        candidateScore[i] = score;
      }
      if (isBetter) {
        int[] temp = bestScore;
        bestScore = candidateScore;
        candidateScore = temp;
        bestMethod = method;
      }
    }
    return bestMethod;
  }

  private int getScore(TypeId target, TypeId concrete) {
    if (target == concrete)
      return 0;
    int best = kUnrelated;
    for (TypeId parent : universe.getParents(concrete)) {
      int score = getScore(target, parent);
      int value = (score == kUnrelated) ? kUnrelated : score + 1;
      best = Math.min(value, best);
    }
    return best;
  }

}
