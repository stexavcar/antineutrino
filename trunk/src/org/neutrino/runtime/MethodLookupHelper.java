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
import org.neutrino.pib.Universe;
import org.neutrino.runtime.lookup.CallInfo;
import org.neutrino.runtime.lookup.DigestedSignature;

public class MethodLookupHelper {

  private static final int kInitialScoreVectorSize = 256;
  private static final int kUnrelated = Integer.MAX_VALUE;
  private static final int kUnspecified = Integer.MAX_VALUE / 2;
  private final Universe universe;
  private int[] bestScore = new int[kInitialScoreVectorSize];
  private int[] candidateScore = new int[kInitialScoreVectorSize];
  private Method bestMethod;
  private int scoreVectorSize = kInitialScoreVectorSize;

  public MethodLookupHelper(Universe universe) {
    this.universe = universe;
  }

  public Method lookupIntrinsic(RValue name, int argc, Stack<RValue> stack) {
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

  public void searchIntrinsics(CallInfo info, Stack<RValue> stack) {
    RValue recv = stack.get(stack.size() - info.getArgumentCount() + info.primaryArgument);
    if (!(recv instanceof RObject))
      return;
    RObject obj = (RObject) recv;
    for (Method method : obj.getImpl().getIntrinsics()) {
      tryCandidate(method, info, stack);
    }
  }

  public void clearTemporaries(CallInfo info) {
    bestMethod = null;
    int argc = info.getArgumentCount();
    if (scoreVectorSize < argc) {
      scoreVectorSize = argc;
      bestScore = new int[argc];
      candidateScore = new int[argc];
    }
    for (int i = 0; i < argc; i++)
      bestScore[i] = Integer.MAX_VALUE;
  }

  public Lambda lookupMethod(CallInfo info, Stack<RValue> stack) {
    clearTemporaries(info);
    searchIntrinsics(info, stack);
    if (bestMethod == null)
      searchApproximateMatches(info, stack);
    if (bestMethod == null) {
      return null;
    } else {
      return new Lambda(bestMethod.module, bestMethod.getBundle());
    }
  }

  public Lambda lookupMethod(RValue name, int argc, Stack<RValue> stack) {
    Method intrinsic = lookupIntrinsic(name, argc, stack);
    if (intrinsic != null)
      return new Lambda(intrinsic.module, intrinsic.getBundle());
    Method result = findPerfectMatch(name, argc, stack);
    if (result == null)
      result = findApproximateMatch(name, argc, stack);
    if (result != null)
      return new Lambda(result.module, result.getBundle());
    else
      return null;
  }

  public Lambda lookupLambda(RValue holder, RValue... args) {
    Frame fakeFrame = new Frame(null, new CodeBundle(new byte[0], Arrays.asList(), 0));
    fakeFrame.stack.push(holder);
    for (RValue arg : args)
      fakeFrame.stack.push(arg);
    return lookupMethod(RString.of("()"), 1 + args.length, fakeFrame.stack);
  }

  private static class MethodCache {

    private int maxArgc = 0;
    private final List<List<Method>> methodsByArgc = new ArrayList<List<Method>>();

    public List<Method> get(int argc) {
      return (maxArgc <= argc) ? null : methodsByArgc.get(argc);
    }

    public void put(int argc, List<Method> values) {
      while (maxArgc <= argc) {
        maxArgc++;
        methodsByArgc.add(null);
      }
      methodsByArgc.set(argc, values);
    }

  }

  private final Map<RValue, MethodCache> methodCache = new HashMap<RValue, MethodCache>();

  private List<Method> getAllMethods(RValue name, int argc) {
    MethodCache cache = methodCache.get(name);
    if (cache == null) {
      cache = new MethodCache();
      methodCache.put(name, cache);
    }
    List<Method> result = cache.get(argc);
    if (result == null) {
      result = new ArrayList<Method>();
      addMethods(result, name, argc, universe);
      cache.put(argc, result);
    }
    return result;
  }

  private void addMethods(List<Method> methods, RValue name,
      int argc, Universe universe) {
    for (Module module : universe.modules.values()) {
      for (Method method : module.methods) {
        Test.Eq test = (Test.Eq) method.signature.get(0).test;
        RValue methodName = test.value;
        if (methodName.equals(name) && (method.signature.size() - 1) == argc)
          methods.add(method);
      }
    }
  }

  private Method findPerfectMatch(RValue name, int argc,
      Stack<RValue> stack) {
    return null;
  }

  private void searchApproximateMatches(CallInfo info, Stack<RValue> stack) {
    for (Module module : universe.modules.values()) {
      for (Method method : module.methods) {
        tryCandidate(method, info, stack);
      }
    }
  }

  private void tryCandidate(Method method, CallInfo info, Stack<RValue> stack) {
    int argc = info.getArgumentCount();
    boolean isBetter = false;
    DigestedSignature sig = method.getDigestedSignature();
    List<DigestedSignature.ParameterTag> tags = sig.getEntries();
    int sigOffset = 0;
    for (int i = 0; i < argc; i++) {
      CallInfo.ArgumentEntry entry = info.entries.get(i);
      while (true) {
        DigestedSignature.ParameterTag tag = tags.get(sigOffset);
        int cmp = entry.tag.compareTo(tag.name);
        if (cmp == 0) {
          RValue value = stack.get(stack.size() - argc + entry.index);
          Test test = method.signature.get(tag.index).test;
          int score = getScore(test, value);
          if (score == kUnrelated)
            return;
          int prevScore = bestScore[entry.index];
          if (prevScore > score) {
            isBetter = true;
          } else if (prevScore < score) {
            return;
          }
          candidateScore[entry.index] = score;
          sigOffset++;
          break;
        } else if ((cmp < 0) && (sigOffset < tags.size() - 1)) {
          sigOffset++;
        } else {
          return;
        }
      }
    }
    if (isBetter) {
      int[] temp = bestScore;
      bestScore = candidateScore;
      candidateScore = temp;
      bestMethod = method;
    }
  }

  private Method findApproximateMatch(RValue name, int argc,
      Stack<RValue> stack) {
    bestMethod = null;
    if (scoreVectorSize < argc) {
      scoreVectorSize = argc;
      bestScore = new int[argc];
      candidateScore = new int[argc];
    }
    for (int i = 0; i < argc; i++)
      bestScore[i] = Integer.MAX_VALUE;
    loop: for (Method method : getAllMethods(name, argc)) {
      List<Pattern> params = method.signature;
      boolean isBetter = false;
      for (int i = 0; i < argc; i++) {
        RValue arg = stack.get(stack.size() - argc + i);
        Test test = params.get(i + 1).test;
        int score;
        switch (test.getType()) {
          case ANY: {
            score = kUnspecified;
            break;
          }
          case EQ: {
            RValue expected = (RValue) ((Test.Eq) test).value;
            if (arg == expected)
              score = 0;
            else
              continue loop;
            break;
          }
          case IS: {
            score = getScore(((Test.Is) test).type, arg.getTypeIds(), 0);
            if (score == kUnrelated)
              continue loop;
            break;
          }
          default:
            throw new RuntimeException();
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

  private int getScore(Test test, RValue value) {
    switch (test.getType()) {
    case ANY:
      return kUnspecified;
    case EQ:
      return ((Test.Eq) test).value.equals(value) ? 0 : kUnrelated;
    case IS:
      return getScore(((Test.Is) test).type, value.getTypeIds(), 0);
    default:
      return kUnrelated;
    }
  }

  private int getScore(RProtocol target, RProtocol[] ids, int dist) {
    int best = kUnrelated;
    for (RProtocol id : ids) {
      int score = getScore(target, id, dist);
      best = Math.min(score, best);
    }
    return best;
  }

  private int getScore(RProtocol target, RProtocol concrete, int dist) {
    if (target == concrete)
      return dist;
    else
      return getScore(target, universe.getParents(concrete), dist + 1);
  }

}
