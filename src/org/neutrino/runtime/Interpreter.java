package org.neutrino.runtime;


import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentLinkedQueue;

import org.javatrino.ast.Method;
import org.javatrino.bytecode.Opcode;
import org.neutrino.pib.CodeBundle;
import org.neutrino.runtime.lookup.CallInfo;

public class Interpreter {

  private static final CallInfo TWO_ARG_CALL = new CallInfo(
      1, Arrays.asList(
          new CallInfo.ArgumentEntry(RInteger.get(0), null, 1),
          new CallInfo.ArgumentEntry(RInteger.get(1), null, 2),
          new CallInfo.ArgumentEntry(RString.of("name"), RString.of("()"), 0)));

  private static final CodeBundle BOTTOM_FRAME_CODE = new CodeBundle(
      new byte[] { Opcode.kCall, 0, 0, 0, Opcode.kTerminate },
      Collections.<Object>emptyList(),
      0);

  private Interpreter() { }

  private final Native.Arguments arguments = new Native.Arguments();

  private static final ConcurrentLinkedQueue<Interpreter> pool = new ConcurrentLinkedQueue<Interpreter>();

  public static RValue run(Lambda lambda, RValue... args) {
    Interpreter inter = pool.poll();
    if (inter == null)
      inter = new Interpreter();
    RValue result = inter.interpret(lambda, args);
    pool.offer(inter);
    return result;
  }

  public static RValue run(CodeBundle code, RValue... args) {
    Interpreter inter = pool.poll();
    if (inter == null)
      inter = new Interpreter();
    RValue result = inter.interpret(code, args);
    pool.offer(inter);
    return result;
  }

  private RValue interpret(CodeBundle code, RValue... args) {
    Frame parent = new Frame(null, BOTTOM_FRAME_CODE);
    for (RValue arg : args)
      parent.stack.push(arg);
    Frame frame = new Frame(parent, code);
    return interpret(frame);
  }

  private RValue interpret(Lambda lambda, RValue... args) {
    return interpret(lambda.getCode(), args);
  }

  private int count = 0;
  @SuppressWarnings("unchecked")
  private RValue interpret(Frame frame) {
    byte[] code = frame.bundle.getCode();
    while (true) {
      int opcode = code[frame.pc];
      // System.out.println(count);
      count++;
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
        int infoIndex = code[frame.pc + 1];
        CallInfo info = (CallInfo) frame.getLiteral(infoIndex);
        Lambda lambda = frame.lookupMethod(info);
        if (lambda == null) {
          info.candidates = null;
          frame.lookupMethod(info);
          throw new InterpreterError.MethodNotFound(frame);
        }
        CodeBundle bundle = lambda.getCode();
        frame = new Frame(frame, bundle);
        code = bundle.getCode();
        break;
      }
      case Opcode.kWithEscape: {
        RContinuation cont = new RContinuation();
        frame.stack.push(cont);
        Lambda lambda = frame.lookupMethod(TWO_ARG_CALL);
        if (lambda == null)
          throw new InterpreterError.MethodNotFound(frame);
        CodeBundle bundle = lambda.getCode();
        frame = new Frame(frame, bundle);
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
        frame.pc += Opcode.kCallSize;
        break;
      }
      case Opcode.kReturnFromApply: {
        RValue value = frame.stack.pop();
        frame = frame.parent.parent;
        code = frame.bundle.getCode();
        frame.stack.pop();
        frame.stack.pop();
        frame.stack.pop();
        frame.stack.pop();
        frame.stack.push(value);
        frame.pc += Opcode.kCallSize;
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
      case Opcode.kCallNative: {
        int index = code[frame.pc + 1];
        Native nathive = (Native) frame.getLiteral(index);
        int argc = code[frame.pc + 2];
        arguments.prepare(frame, argc);
        RValue value = nathive.call(arguments);
        frame = arguments.getFrame();
        code = frame.bundle.getCode();
        if (value != null) {
          frame.stack.push(value);
          frame.pc += Opcode.kCallSize;
        }
        break;
      }
      case Opcode.kGlobal: {
        int index = code[frame.pc + 1];
        RValue name = (RValue) frame.getLiteral(index);
        RValue value = frame.bundle.module.getGlobal(name);
        if (value == null) {
          frame.bundle.module.getGlobal(name);
          throw new InterpreterError.UndefinedGlobal(name, frame);
        }
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
      default:
        assert false: "Unexpected opcode " + opcode;
        return null;
      }
    }
  }

}
