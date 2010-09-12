package org.neutrino.runtime;


import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Opcode;

public class Interpreter {

  public RValue interpret(CodeBundle code) {
    Frame frame = new Frame(null, code);
    interpret(frame);
    return frame.stack.pop();
  }

  private void interpret(Frame frame) {
    while (true) {
      int opcode = frame.code[frame.pc];
      switch (opcode) {
      case Opcode.kNumber:
        int value = frame.code[frame.pc + 1];
        frame.stack.push(new RInteger(value));
        frame.pc += 2;
        break;
      case Opcode.kLambda:
        int index = frame.code[frame.pc + 1];
        CodeBundle bundle = (CodeBundle) frame.literals.get(index);
        frame.stack.push(new RLambda(bundle));
        frame.pc += 2;
        break;
      case Opcode.kCall:

        return;
      case Opcode.kReturn:
        Frame parent = frame.parent;
        if (parent == null)
          return;
        frame = parent;
        frame.pc += 1;
        break;
      default:
        assert false: "Unexpected opcode " + opcode;
        return;
      }
    }
  }

}
