package org.neutrino.runtime;


import java.util.Collections;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Opcode;

public class Interpreter {

  private static final RValue[] NO_VALUES = new RValue[0];

  private static final CodeBundle BOTTOM_FRAME_CODE = new CodeBundle(
      new byte[] { Opcode.kCall, 0, 0, Opcode.kTerminate },
      Collections.<Object>emptyList(),
      0);

  private final Native.Arguments arguments = new Native.Arguments();

  public RValue interpret(Module module, CodeBundle code, RValue... args) {
    Frame parent = new Frame(null, BOTTOM_FRAME_CODE, null, null);
    for (RValue arg : args)
      parent.stack.push(arg);
    Frame frame = new Frame(parent, code, null, module);
    return interpret(frame);
  }

  public RValue interpret(RLambda lambda, RValue... args) {
    return interpret(lambda.getModule(), lambda.getCode(), args);
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
        RValue[] outer = NO_VALUES;
        if (outc > 0) {
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
        frame = frame.parent;
        int argc = frame.code[frame.pc + 2];
        for (int i = 0; i < argc; i++)
          frame.stack.pop();
        frame.stack.push(value);
        frame.pc += 3;
        break;
      }
      case Opcode.kTerminate: {
        RValue value = frame.stack.pop();
        return value;
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
        RValue value = frame.module.getUniverse().getGlobal(name, this);
        assert value != null: "Undefined global " + name;
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
      case Opcode.kNew: {
        int protoIndex = frame.code[frame.pc + 1];
        int outc = frame.code[frame.pc + 2];
        RProtocol proto = (RProtocol) frame.getLiteral(protoIndex);
        RValue[] outer = NO_VALUES;
        if (outc > 0) {
          outer = new RValue[outc];
          for (int i = 0; i < outc; i++)
            outer[outc - i - 1] = frame.stack.pop();
        }
        frame.stack.push(new RObject(proto, outer));
        frame.pc += 3;
        break;
      }
      case Opcode.kField: {
        int index = frame.code[frame.pc + 1];
        RObject obj = (RObject) frame.parent.getArgument(1, 0);
        frame.stack.push(obj.getField(index));
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
