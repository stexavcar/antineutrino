package org.neutrino.runtime;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import org.javatrino.ast.Method;
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

  /**
   * Looks for the best method among intrinsics.
   */
  public void searchIntrinsics(CallInfo info, Stack<RValue> stack) {
    RValue recv = stack.get(stack.size() - info.getArgumentCount() + info.primaryArgument);
    if (!(recv instanceof RObject))
      return;
    RObject obj = (RObject) recv;
    for (Method method : obj.getImpl().getIntrinsics()) {
      tryCandidate(method, info, stack, 0);
    }
  }

  /**
   * Looks for the best method in the context.
   */
  private void searchContext(CallInfo info, Stack<RValue> stack) {
    for (Method method : getApplicableMethods(info.getArgumentCount())) {
      tryCandidate(method, info, stack, 1);
    }
  }

  public Lambda lookupMethod(CallInfo info, Stack<RValue> stack) {
    clearTemporaries(info);
    searchIntrinsics(info, stack);
    searchContext(info, stack);
    if (bestMethod == null) {
      return null;
    } else {
      return new Lambda(bestMethod.module, bestMethod.getBundle());
    }
  }

  public Lambda lookupLambda(RValue... args) {
    Frame fakeFrame = new Frame(null, new CodeBundle(new byte[0], Arrays.asList(), 0));
    fakeFrame.stack.push(RString.of("()"));
    List<CallInfo.ArgumentEntry> entries = new ArrayList<CallInfo.ArgumentEntry>();
    for (int i = 0; i < args.length; i++) {
      fakeFrame.stack.push(args[i]);
      entries.add(new CallInfo.ArgumentEntry(RInteger.get(i), null, i + 1));
    }
    entries.add(new CallInfo.ArgumentEntry(RString.of("name"), null, 0));
    return lookupMethod(new CallInfo(0, entries), fakeFrame.stack);
  }

  private final Map<Integer, List<Method>> applicable = new HashMap<Integer, List<Method>>();

  /**
   * Returns a list of all methods that will accept the given number of arguments.
   */
  private List<Method> getApplicableMethods(int argc) {
    List<Method> result = applicable.get(argc);
    if (result == null) {
      result = new ArrayList<Method>();
      for (Module module : universe.modules.values()) {
        for (Method method : module.methods) {
          if (method.isApplicable(argc)) {
            result.add(method);
          }
        }
      }
      applicable.put(argc, result);
    }
    return result;
  }

  private void tryCandidate(Method method, CallInfo info, Stack<RValue> stack,
      int dist) {
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
          int score = getScore(test, value, dist);
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

  private int getScore(Test test, RValue value, int dist) {
    switch (test.getType()) {
    case ANY:
      return kUnspecified;
    case EQ:
      return ((Test.Eq) test).value.equals(value) ? dist : kUnrelated;
    case IS:
      return getScore(((Test.Is) test).type, value.getTypeIds(), dist);
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
