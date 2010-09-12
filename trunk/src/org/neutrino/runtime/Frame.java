package org.neutrino.runtime;

import java.util.List;
import java.util.Stack;

import org.neutrino.pib.CodeBundle;

public class Frame {

  public int pc = 0;
  public final Stack<RValue> stack = new Stack<RValue>();
  public final byte[] code;
  public final List<Object> literals;
  public final Frame parent;

  public Frame(Frame parent, CodeBundle code) {
    this.parent = parent;
    this.code = code.getCode();
    this.literals = code.getLiterals();
  }

}