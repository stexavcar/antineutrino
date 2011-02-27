package org.neutrino.runtime;


import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.javatrino.ast.Method;
import org.javatrino.bytecode.Opcode;
import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Universe;

public class Interpreter {

  private static final CodeBundle BOTTOM_FRAME_CODE = new CodeBundle(
      new byte[] { Opcode.kCall, 0, 0, 0, Opcode.kTerminate },
      Collections.<Object>emptyList(),
      0);

  private final Native.Arguments arguments = new Native.Arguments();

  public RValue interpret(Universe universe, CodeBundle code, RValue... args) {
    Frame parent = new Frame(null, null, BOTTOM_FRAME_CODE);
    for (RValue arg : args)
      parent.stack.push(arg);
    Frame frame = new Frame(parent, null, code);
    return interpret(universe, frame);
  }

  public RValue interpret(Universe universe, Lambda lambda, RValue... args) {
    return interpret(universe, lambda.getCode(), args);
  }

  static int count = 0;

  @SuppressWarnings("unchecked")
  private RValue interpret(Universe universe, Frame frame) {
    byte[] code = frame.bundle.getCode();
    while (true) {
//      System.out.println(count++);
      int opcode = code[frame.pc];
      switch (opcode) {
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
      case Opcode.kCall: {
        int nameIndex = code[frame.pc + 1];
        String name = (String) frame.getLiteral(nameIndex);
        int argc = code[frame.pc + 2];
        RValue self = frame.getArgument(argc, 0);
        Lambda lambda = frame.lookupMethod(universe, name, argc);
        if (lambda == null) {
          frame.lookupMethod(universe, name, argc);
          throw new InterpreterError.MethodNotFound(frame, universe);
        }
        CodeBundle bundle = lambda.getCode();
        frame = new Frame(frame, self, bundle);
        code = bundle.getCode();
        break;
      }
      case Opcode.kWith1cc: {
        String name = "()";
        int argc = code[frame.pc + 2];
        RContinuation cont = new RContinuation();
        frame.stack.push(cont);
        RValue self = frame.getArgument(argc, 0);
        Lambda lambda = frame.lookupMethod(universe, name, argc);
        if (lambda == null)
          throw new InterpreterError.MethodNotFound(frame, universe);
        CodeBundle bundle = lambda.getCode();
        frame = new Frame(frame, self, bundle);
        frame.marker = cont;
        code = bundle.getCode();
        break;
      }
      case Opcode.kWithEscape: {
        String name = "()";
        int argc = code[frame.pc + 2];
        RContinuation cont = new RContinuation();
        RValue self = frame.stack.peek();
        frame.stack.push(cont);
        Lambda lambda = frame.lookupMethod(universe, name, argc);
        if (lambda == null)
          throw new InterpreterError.MethodNotFound(frame, universe);
        CodeBundle bundle = lambda.getCode();
        frame = new Frame(frame, self, bundle);
        code = bundle.getCode();
        frame.marker = cont;
        break;
      }
      case Opcode.kReturn: {
        RValue value = frame.stack.pop();
        assert frame.stack.size() == frame.getLocalCount();
        frame = frame.parent;
        code = frame.bundle.getCode();
        int argc = code[frame.pc + 2];
        for (int i = 0; i < argc; i++)
          frame.stack.pop();
        frame.stack.push(value);
        frame.pc += Opcode.kCallSize + argc;
        break;
      }
      case Opcode.kReturnFromApply: {
        RValue value = frame.stack.pop();
        frame = frame.parent.parent;
        code = frame.bundle.getCode();
        frame.stack.pop();
        frame.stack.pop();
        frame.stack.pop();
        frame.stack.push(value);
        int argc = code[frame.pc + 2];
        frame.pc += Opcode.kCallSize + argc;
        break;
      }
      case Opcode.kTerminate: {
        RValue value = frame.stack.pop();
        return value;
      }
      case Opcode.kLiteral: {
        int index = code[frame.pc + 1];
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
      case Opcode.kDup: {
        frame.stack.push(frame.stack.peek());
        frame.pc += 1;
        break;
      }
      case Opcode.kArgument: {
        int index = code[frame.pc + 1];
        RValue value = frame.parent.peekArgument(index);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kOuter: {
        int index = code[frame.pc + 1];
        RFieldKey field = (RFieldKey) frame.getLiteral(index);
        frame.stack.push(((RObject) frame.holder).getField(field));
        frame.pc += 2;
        break;
      }
      case Opcode.kCallNative: {
        int index = code[frame.pc + 1];
        Native nathive = (Native) frame.getLiteral(index);
        int argc = code[frame.pc + 2];
        arguments.prepare(frame, argc, universe);
        RValue value = nathive.call(arguments);
        frame = arguments.getFrame();
        code = frame.bundle.getCode();
        if (value != null) {
          frame.stack.push(value);
          int targargc = code[frame.pc + 2];
          frame.pc += Opcode.kCallSize + targargc;
        }
        break;
      }
      case Opcode.kGlobal: {
        int index = code[frame.pc + 1];
        Object name = frame.getLiteral(index);
        RValue value = universe.getGlobal(name, this);
        if (value == null)
          throw new InterpreterError.UndefinedGlobal(name, frame, universe);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kStoreLocal: {
        int index = code[frame.pc + 1];
        frame.setLocal(index, frame.stack.pop());
        frame.pc += 2;
        break;
      }
      case Opcode.kLocal: {
        int index = code[frame.pc + 1];
        RValue value = frame.getLocal(index);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kNew: {
        int protoIndex = code[frame.pc + 1];
        RProtocol proto = (RProtocol) frame.getLiteral(protoIndex);
        int fieldIndex = code[frame.pc + 2];
        List<RFieldKey> fields = (List<RFieldKey>) frame.getLiteral(fieldIndex);
        Map<RFieldKey, RValue> outer = Collections.<RFieldKey, RValue>emptyMap();
        if (fields.size() > 0) {
          outer = new HashMap<RFieldKey, RValue>();
          for (int i = fields.size() - 1; i >= 0; i--)
            outer.put(fields.get(i), frame.stack.pop());
        }
        frame.stack.push(new RObject(proto, outer));
        frame.pc += 3;
        break;
      }
      case Opcode.kNewArray: {
        int elmc = code[frame.pc + 1];
        RValue[] elms = new RValue[elmc];
        for (int i = 0; i < elmc; i++)
          elms[elmc - i - 1] = frame.stack.pop();
        frame.stack.push(new RPrimitiveArray(Arrays.asList(elms)));
        frame.pc += 2;
        break;
      }
      case Opcode.kGetter: {
        int index = code[frame.pc + 1];
        RFieldKey field = (RFieldKey) frame.getLiteral(index);
        RObject obj = (RObject) frame.parent.getArgument(1, 0);
        frame.stack.push(obj.getField(field));
        frame.pc += 2;
        break;
      }
      case Opcode.kSetter: {
        int index = code[frame.pc + 1];
        RFieldKey field = (RFieldKey) frame.getLiteral(index);
        RObject obj = (RObject) frame.parent.getArgument(2, 0);
        RValue value = frame.parent.getArgument(2, 1);
        obj.setField(field, value);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kNewObject: {
        RObject obj = new RObject();
        frame.stack.push(obj);
        frame.pc += 1;
        break;
      }
      case Opcode.kSetField: {
        int index = code[frame.pc + 1];
        RFieldKey field = (RFieldKey) frame.getLiteral(index);
        RValue value = (RValue) frame.stack.pop();
        RObject obj = (RObject) frame.stack.pop();
        obj.setField(field, value);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kGetField: {
        int index = code[frame.pc + 1];
        RFieldKey field = (RFieldKey) frame.getLiteral(index);
        RObject obj = (RObject) frame.stack.pop();
        RValue value = obj.getField(field);
        frame.stack.push(value);
        frame.pc += 2;
        break;
      }
      case Opcode.kAddIntrinsics: {
        int index = code[frame.pc + 1];
        List<Method> methods = (List<Method>) frame.getLiteral(index);
        RObject obj = (RObject) frame.stack.peek();
        obj.addIntrinsics(methods);
        frame.pc += 2;
        break;
      }
      case Opcode.kTagWithProtocols: {
        int count = code[frame.pc + 1];
        RObject obj = (RObject) frame.stack.get(frame.stack.size() - 1 - count);
        for (int i = 0; i < count; i++)
          obj.addProtocol((RProtocol) frame.stack.pop());
        frame.pc += 2;
        break;
      }
      // Structural opcodes without execution semantics.
      case Opcode.kDefineLocal: {
        frame.pc += 4;
        break;
      }
      case Opcode.kBlock: {
        int argc = code[frame.pc + 1];
        frame.pc += 2 + argc;
        break;
      }
      default:
        assert false: "Unexpected opcode " + opcode;
        return null;
      }
    }
  }

}
