package org.neutrino.runtime;

import java.util.Stack;

import org.neutrino.pib.CodeBundle;

public class Frame {

  public int pc = 0;
  public Stack<RValue> stack = new Stack<RValue>();
  public final CodeBundle bundle;
  public final Frame parent;
  public final RValue holder;
  public RContinuation marker;

  public Frame(Frame parent, RValue holder, CodeBundle code) {
    this.parent = parent;
    this.holder = holder;
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

  public Lambda lookupMethod(RValue name, int argc) {
    return bundle.module.universe.lookupMethod(name, argc, this.stack);
  }

  public RValue peekArgument(int index) {
    return stack.get(stack.size() - index);
  }

  public Object getLiteral(int index) {
    return bundle.getLiterals().get(index);
  }

  public void setLocal(int index, RValue value) {
    stack.set(index, value);
  }

  public RValue getLocal(int index) {
    return stack.get(index);
  }

}
