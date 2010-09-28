package org.neutrino.pib;

import org.neutrino.pib.Opcode.ArgumentType;
import org.neutrino.pib.Opcode.OpcodeInfo;
import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

import java.util.List;

@Growable(CodeBundle.TAG)
public class CodeBundle implements ISeedable {

  public static final String TAG = "org::neutrino::pib::CodeBundle";

  public @SeedMember byte[] code;
  public @SeedMember List<Object> literals;

  public CodeBundle(byte[] code, List<Object> literals) {
    this.code = code;
    this.literals = literals;
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
      ArgumentType[] args = info.getArguments();
      for (int i = 0; i < args.length; i++) {
        if (i > 0)
          buf.append(", ");
        int value = code[pc + 1 + i];
        ArgumentType type = args[i];
        switch (type) {
        case LITERAL:
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
