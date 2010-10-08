package org.neutrino.pib;

import java.util.List;

import org.neutrino.pib.Opcode.ArgType;
import org.neutrino.pib.Opcode.OpcodeInfo;
import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(CodeBundle.TAG)
public class CodeBundle implements ISeedable {

  static final String TAG = "org::neutrino::pib::CodeBundle";

  public @SeedMember byte[] code;
  public @SeedMember List<Object> literals;
  public @SeedMember int localCount;
  public @SeedMember String fileName;

  public CodeBundle(byte[] code, List<Object> literals, int localCount,
      String fileName) {
    this.code = code;
    this.literals = literals;
    this.localCount = localCount;
    this.fileName = fileName;
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
