package org.neutrino.runtime;


import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Opcode;

public class Interpreter {

  public RValue interpret(Module module, CodeBundle code) {
    Frame frame = new Frame(null, code, module);
    return interpret(frame);
  }

  public RValue interpret(RLambda lambda) {
    return interpret(lambda.getModule(), lambda.getCode());
  }

  private RValue interpret(Frame frame) {
    while (true) {
      int opcode = frame.code[frame.pc];
      switch (opcode) {
      case Opcode.kNumber: {
        int value = frame.code[frame.pc + 1];
        frame.stack.push(new RInteger(value));
        frame.pc += 2;
        break;
      }
      case Opcode.kNull:
        frame.stack.push(RNull.getInstance());
        frame.pc += 1;
        break;
      case Opcode.kTrue:
        frame.stack.push(RBoolean.getTrue());
        frame.pc += 1;
        break;
      case Opcode.kLambda: {
        int index = frame.code[frame.pc + 1];
        CodeBundle bundle = (CodeBundle) frame.literals.get(index);
        frame.stack.push(new RLambda(frame.module, bundle));
        frame.pc += 2;
        break;
      }
      case Opcode.kCall: {
        int nameIndex = frame.code[frame.pc + 1];
        String name = (String) frame.literals.get(nameIndex);
        int argc = frame.code[frame.pc + 2];
        RLambda lambda = frame.lookupMethod(name, argc);
        frame = new Frame(frame, lambda.getCode(), lambda.getModule());
        break;
      }
      case Opcode.kReturn: {
        RValue value = frame.stack.pop();
        assert frame.stack.isEmpty();
        Frame parent = frame.parent;
        if (parent == null) {
          return value;
        }
        frame = parent;
        int argc = frame.code[frame.pc + 2];
        for (int i = 0; i < argc; i++)
          frame.stack.pop();
        frame.stack.push(value);
        frame.pc += 3;
        break;
      }
      case Opcode.kPush: {
        int index = frame.code[frame.pc + 1];
        RValue value = (RValue) frame.literals.get(index);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kCallNative: {
        int index = frame.code[frame.pc + 1];
        Native nathive = (Native) frame.literals.get(index);
        RValue value = nathive.call(frame.parent);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      default:
        assert false: "Unexpected opcode " + opcode;
        return null;
      }
    }
  }

}
