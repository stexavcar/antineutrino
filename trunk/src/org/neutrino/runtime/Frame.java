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

  public Frame(Frame parent, CodeBundle code, Module module) {
    this.parent = parent;
    this.code = code.getCode();
    this.literals = code.getLiterals();
    this.module = module;
  }

  public RLambda lookupMethod(String name, int argc) {
    for (RMethod method : module.methods) {
      if (method.getName().equals(name)) {
        return new RLambda(module, method.getCode());
      }
    }
    return null;
  }

}