package org.neutrino.runtime;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import org.javatrino.ast.Method;
import org.javatrino.ast.Pattern;
import org.javatrino.ast.Test;
import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Parameter;
import org.neutrino.pib.Universe;

public class MethodLookupHelper {

  private static final int kInitialScoreVectorSize = 256;
  private static final int kUnrelated = Integer.MAX_VALUE;
  private static final int kUnspecified = Integer.MAX_VALUE / 2;
  private final Universe universe;
  private int[] bestScore = new int[kInitialScoreVectorSize];
  private int[] candidateScore = new int[kInitialScoreVectorSize];
  private RMethod bestMethod;
  private int scoreVectorSize = kInitialScoreVectorSize;

  public MethodLookupHelper(Universe universe) {
    this.universe = universe;
  }

  public Method lookupIntrinsic(String name, int argc, Stack<RValue> stack) {
    RValue recv = stack.get(stack.size() - argc);
    if (!(recv instanceof RObject))
      return null;
    RObject obj = (RObject) recv;
    List<Method> intrinsics = obj.getImpl().getIntrinsics();
    for (Method method : intrinsics) {
      List<Pattern> sig = method.signature;
      Pattern namePattern = sig.get(0);
      if (name.equals(((Test.Eq) namePattern.test).value)) {
        return method;
      }
    }
    return null;
  }

  public Lambda lookupMethod(String name, int argc, Stack<RValue> stack) {
    Method intrinsic = lookupIntrinsic(name, argc, stack);
    if (intrinsic != null)
      return new Lambda(intrinsic.module, intrinsic.getBundle());
    RMethod result = findPerfectMatch(name, argc, stack);
    if (result == null)
      result = findApproximateMatch(name, argc, stack);
    if (result != null)
      return new Lambda(result.getOrigin(), result.getCode());
    else
      return null;
  }

  public Lambda lookupLambda(RObject holder, RValue... args) {
    Frame fakeFrame = new Frame(null, null, new CodeBundle(new byte[0], Arrays.asList(), 0));
    fakeFrame.stack.push(holder);
    for (RValue arg : args)
      fakeFrame.stack.push(arg);
    return lookupMethod("()", 1 + args.length, fakeFrame.stack);
  }

  private static class MethodCache {

    private int maxArgc = 0;
    private final List<List<RMethod>> methodsByArgc = new ArrayList<List<RMethod>>();

    public List<RMethod> get(int argc) {
      return (maxArgc <= argc) ? null : methodsByArgc.get(argc);
    }

    public void put(int argc, List<RMethod> values) {
      while (maxArgc <= argc) {
        maxArgc++;
        methodsByArgc.add(null);
      }
      methodsByArgc.set(argc, values);
    }

  }

  private final Map<String, MethodCache> methodCache = new HashMap<String, MethodCache>();

  private List<RMethod> getAllMethods(String name, int argc) {
    MethodCache cache = methodCache.get(name);
    if (cache == null) {
      cache = new MethodCache();
      methodCache.put(name, cache);
    }
    List<RMethod> result = cache.get(argc);
    if (result == null) {
      result = new ArrayList<RMethod>();
      addMethods(result, name, argc, universe);
      cache.put(argc, result);
    }
    return result;
  }

  private void addMethods(List<RMethod> methods, String name,
      int argc, Universe universe) {
    for (Module module : universe.modules.values()) {
      for (RMethod method : module.methods) {
        if (method.name.equals(name) && method.getParameters().size() == argc)
          methods.add(method);
      }
    }
  }

  private RMethod findPerfectMatch(String name, int argc,
      Stack<RValue> stack) {
    loop: for (RMethod method : getAllMethods(name, argc)) {
      List<Parameter> params = method.getParameters();
      for (int i = 0; i < argc; i++) {
        TypeId paramType = params.get(i).getTypeId();
        if (paramType == null)
          paramType = TypeId.get("Object");
        RValue arg = stack.get(stack.size() - argc + i);
        boolean foundIt = false;
        for (TypeId argType : arg.getTypeIds()) {
          if (argType == paramType) {
            foundIt = true;
            break;
          }
        }
        if (!foundIt)
          continue loop;
      }
      return method;
    }
    return null;
  }

  private RMethod findApproximateMatch(String name, int argc,
      Stack<RValue> stack) {
    bestMethod = null;
    if (scoreVectorSize < argc) {
      scoreVectorSize = argc;
      bestScore = new int[argc];
      candidateScore = new int[argc];
    }
    for (int i = 0; i < argc; i++)
      bestScore[i] = Integer.MAX_VALUE;
    loop: for (RMethod method : getAllMethods(name, argc)) {
      List<Parameter> params = method.getParameters();
      boolean isBetter = false;
      for (int i = 0; i < argc; i++) {
        TypeId paramType = params.get(i).getTypeId();
        int score;
        if (paramType == null) {
          score = kUnspecified;
        } else {
          RValue arg = stack.get(stack.size() - argc + i);
          score = getScore(paramType, arg.getTypeIds(), 0);
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

  private int getScore(TypeId target, Iterable<TypeId> ids, int dist) {
    int best = kUnrelated;
    for (TypeId id : ids) {
      int score = getScore(target, id, dist);
      best = Math.min(score, best);
    }
    return best;
  }

  private int getScore(TypeId target, TypeId concrete, int dist) {
    if (target == concrete)
      return dist;
    else
      return getScore(target, universe.getParents(concrete), dist + 1);
  }

}
