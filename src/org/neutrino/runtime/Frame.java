package org.neutrino.runtime;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;

import java.util.Stack;

public class Frame {

  public int pc = 0;
  public final Stack<RValue> stack = new Stack<RValue>();
  public final byte[] code;
  public final CodeBundle bundle;
  public final Module module;
  public final Frame parent;
  public final RValue[] outer;

  public Frame(Frame parent, CodeBundle code, RValue[] outer, Module module) {
    this.parent = parent;
    this.code = code.getCode();
    this.bundle = code;
    this.outer = outer;
    this.module = module;
    for (int i = 0; i < code.localCount; i++)
      stack.push(RNull.getInstance());
  }

  public int getLocalCount() {
    return bundle.localCount;
  }

  public RValue getArgument(int argc, int index) {
    return stack.get(stack.size() - argc + index);
  }

  public RLambda lookupMethod(String name, int argc) {
    return module.lookupMethod(name, argc, this);
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
