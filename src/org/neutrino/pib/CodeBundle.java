package org.neutrino.pib;

import java.util.List;

import org.neutrino.pib.Opcode.ArgType;
import org.neutrino.pib.Opcode.OpcodeInfo;
import org.neutrino.plankton.Store;

public class CodeBundle {

  public @Store byte[] code;
  public @Store List<Object> literals;
  public @Store int localCount;
  public @Store String fileName;
  public @Store int rootOffset;

  public CodeBundle(byte[] code, List<Object> literals, int localCount,
      String fileName, int rootOffset) {
    this.code = code;
    this.literals = literals;
    this.localCount = localCount;
    this.fileName = fileName;
    this.rootOffset = rootOffset;
  }

  public CodeBundle() { }

  public byte[] getCode() {
    return this.code;
  }

  public List<Object> getLiterals() {
    return this.literals;
  }

  @Override
  public String toString() {
    StringBuilder buf = new StringBuilder().append("[");
    for (int pc = 0; pc < code.length;) {
      if (pc > 0)
        buf.append(" ");
      int op = code[pc];
      OpcodeInfo info = Opcode.getInfo(op);
      assert info != null : "Unknown opcode " + op;
      buf.append(info.getName()).append("(");
      ArgType[] args = info.getArguments();
      for (int i = 0; i < args.length; i++) {
        if (i > 0)
          buf.append(", ");
        int value = code[pc + 1 + i];
        ArgType type = args[i];
        switch (type) {
        case LIT:
          buf.append(literals.get(value));
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
