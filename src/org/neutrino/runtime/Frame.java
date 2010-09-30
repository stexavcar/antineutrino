package org.neutrino.runtime;

import java.util.List;
import java.util.Stack;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;

public class Frame {

  public int pc = 0;
  public final Stack<RValue> stack = new Stack<RValue>();
  public final byte[] code;
  public final List<Object> literals;
  public final Module module;
  public final Frame parent;
  public final RValue[] outer;

  public Frame(Frame parent, CodeBundle code, RValue[] outer, Module module) {
    this.parent = parent;
    this.code = code.getCode();
    this.literals = code.getLiterals();
    this.outer = outer;
    this.module = module;
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

}