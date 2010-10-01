package org.neutrino.runtime;


import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Opcode;

public class Interpreter {

  private final Native.Arguments arguments = new Native.Arguments();

  public RValue interpret(Module module, CodeBundle code) {
    Frame frame = new Frame(null, code, null, module);
    return interpret(frame);
  }

  public RValue interpret(RLambda lambda) {
    return interpret(lambda.getModule(), lambda.getCode());
  }

  private String getMethodName(String name, int argc, Frame frame) {
    StringBuilder buf = new StringBuilder().append(name).append("(");
    for (int i = 0; i < argc; i++) {
      if (i > 0) buf.append(", ");
      RValue value = frame.getArgument(argc, i);
      buf.append(value.getTypeId());
    }
    return buf.append(")").toString();
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
      case Opcode.kFalse:
        frame.stack.push(RBoolean.getFalse());
        frame.pc += 1;
        break;
      case Opcode.kLambda: {
        int index = frame.code[frame.pc + 1];
        int outc = frame.code[frame.pc + 2];
        CodeBundle bundle = (CodeBundle) frame.getLiteral(index);
        RValue[] outer;
        if (outc == 0) {
          outer = null;
        } else {
          outer = new RValue[outc];
          for (int i = 0; i < outc; i++)
            outer[outc - i - 1] = frame.stack.pop();
        }
        frame.stack.push(new RLambda(frame.module, bundle, outer));
        frame.pc += 3;
        break;
      }
      case Opcode.kCall: {
        int nameIndex = frame.code[frame.pc + 1];
        String name = (String) frame.getLiteral(nameIndex);
        int argc = frame.code[frame.pc + 2];
        RLambda lambda = frame.lookupMethod(name, argc);
        assert lambda != null : "Method " + getMethodName(name, argc, frame) + " not found.";
        frame = new Frame(frame, lambda.getCode(), lambda.getOuter(), lambda.getModule());
        break;
      }
      case Opcode.kReturn: {
        RValue value = frame.stack.pop();
        assert frame.stack.size() == frame.getLocalCount();
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
        RValue value = (RValue) frame.getLiteral(index);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kPop: {
        frame.stack.pop();
        frame.pc += 1;
        break;
      }
      case Opcode.kArgument: {
        int index = frame.code[frame.pc + 1];
        RValue value = frame.parent.peekArgument(index);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kOuter: {
        int index = frame.code[frame.pc + 1];
        frame.stack.push(frame.outer[index]);
        frame.pc += 2;
        break;
      }
      case Opcode.kCallNative: {
        int index = frame.code[frame.pc + 1];
        Native nathive = (Native) frame.getLiteral(index);
        int argc = frame.code[frame.pc + 2];
        arguments.prepare(frame.parent, argc);
        RValue value = nathive.call(arguments);
        frame.stack.push(value);
        frame.pc += 3;
        break;
      }
      case Opcode.kGlobal: {
        int index = frame.code[frame.pc + 1];
        String name = (String) frame.getLiteral(index);
        RValue value = frame.module.getGlobal(name, this);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kStoreLocal: {
        int index = frame.code[frame.pc + 1];
        frame.setLocal(index, frame.stack.pop());
        frame.pc += 2;
        break;
      }
      case Opcode.kLocal: {
        int index = frame.code[frame.pc + 1];
        RValue value = frame.getLocal(index);
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
