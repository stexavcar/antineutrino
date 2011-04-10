package org.neutrino.runtime;

import java.util.Stack;

import org.neutrino.pib.CodeBundle;
import org.neutrino.runtime.lookup.CallInfo;

public class Frame {

  public int pc = 0;
  public Stack<RValue> stack = new Stack<RValue>();
  public final CodeBundle bundle;
  public final Frame parent;
  public RContinuation marker;

  public Frame(Frame parent, CodeBundle code) {
    this.parent = parent;
    this.bundle = code;
    for (int i = 0; i < code.getLocalCount(); i++)
      stack.push(RNull.getInstance());
  }

  public int getLocalCount() {
    return bundle.getLocalCount();
  }

  public RValue getArgument(int argc, int index) {
    return stack.get(stack.size() - argc + index);
  }

  public Lambda lookupMethod(CallInfo info) {
    return bundle.module.universe.lookupMethod(info, this.stack);
  }

  public RValue peekArgument(int index) {
    return stack.get(stack.size() - index);
  }

  public Object getLiteral(int index) {
    return bundle.getLiterals().get(index & 0xFF);
  }

  public void setLocal(int index, RValue value) {
    stack.set(index, value);
  }

  public RValue getLocal(int index) {
    return stack.get(index);
  }

}
