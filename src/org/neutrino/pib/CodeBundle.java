package org.neutrino.pib;

import java.util.List;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Symbol;
import org.javatrino.bytecode.BytecodeCompiler;
import org.javatrino.bytecode.Opcode;
import org.javatrino.bytecode.Opcode.ArgType;
import org.javatrino.bytecode.Opcode.OpcodeInfo;
import org.neutrino.plankton.Store;

public class CodeBundle {

  public @Store byte[] oldCode;
  public @Store List<Object> oldLiterals;
  public @Store int oldLocalCount;
  public @Store String fileName;
  public @Store int rootOffset;
  public @Store Expression body;
  public @Store List<Symbol> params;
  
  private byte[] newCode;
  private List<Object> newLiterals;
  private int newLocalCount = -1;

  public CodeBundle(byte[] code, List<Object> literals, int oldLocalCount,
      String fileName, int rootOffset, Expression body, List<Symbol> params) {
    this.oldCode = code;
    this.oldLiterals = literals;
    this.oldLocalCount = oldLocalCount;
    this.fileName = fileName;
    this.rootOffset = rootOffset;
    this.body = body;
    this.params = params;
  }

  public CodeBundle() { }

  public byte[] getOldCode() {
    return this.oldCode;
  }

  public List<Object> getOldLiterals() {
    return this.oldLiterals;
  }

  public byte[] getCode() {
    if (newCode != null) {
      return newCode;
    } else if (body == null) {
      return oldCode;
    } else {
      ensureCompiled();
      return newCode;
    }
  }
  
  public int getLocalCount() {
    if (newLocalCount != -1) {
      return newLocalCount;
    } else if (body == null) {
      return oldLocalCount;
    } else {
      ensureCompiled();
      return newLocalCount;
    }
  }


  public List<Object> getLiterals() {
    if (newLiterals != null) {
      return newLiterals;
    } else if (body == null) {
      return oldLiterals;
    } else {
      ensureCompiled();
      return newLiterals;
    }
  }

  private void ensureCompiled() {
    if (newCode == null) {
      BytecodeCompiler.Result result = BytecodeCompiler.compile(body, params);
      this.newCode = result.code;
      this.newLiterals = result.literals;
      this.newLocalCount = result.localCount;
    }
  }

  @Override
  public String toString() {
    StringBuilder buf = new StringBuilder().append("[");
    for (int pc = 0; pc < oldCode.length;) {
      if (pc > 0)
        buf.append(" ");
      int op = oldCode[pc];
      OpcodeInfo info = Opcode.getInfo(op);
      assert info != null : "Unknown opcode " + op;
      buf.append(info.getName()).append("(");
      ArgType[] args = info.getArguments();
      for (int i = 0; i < args.length; i++) {
        if (i > 0)
          buf.append(", ");
        int value = oldCode[pc + 1 + i];
        ArgType type = args[i];
        switch (type) {
        case LIT:
          buf.append(oldLiterals.get(value));
          break;
        default:
          buf.append(Integer.toString(value));
          break;
        }
      }
      buf.append(")");
      pc += info.getSize();
    }
    return buf.append("]").toString();
  }

}
