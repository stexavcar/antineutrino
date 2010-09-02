package org.neutrino.runtime;

import org.neutrino.pib.Opcode;

public class Interpreter {

  public Object interpret(byte[] code) {
    return interpret(0, code);
  }

  private Object interpret(int pc, byte[] code) {
    int opcode = code[pc];
    switch (opcode) {
    case Opcode.kNumber:
      int value = code[pc + 1];
      return value;
    default:
      assert false: "Unexpected opcode " + opcode;
      return null;
    }
  }

}
