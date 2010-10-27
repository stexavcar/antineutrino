package org.neutrino.runtime;

import org.neutrino.pib.Opcode;

import java.io.PrintStream;
import java.util.ArrayList;

public class InterpreterError extends RuntimeException {

  private final Frame frame;
  private StackTraceElement[] trace;

  public InterpreterError(Frame frame) {
    this.frame = frame;
  }

  @Override
  public StackTraceElement[] getStackTrace() {
    if (trace == null)
      trace = buildStackTrace();
    return trace;
  }

  private String renderValueType(Frame frame, RValue value) {
    RProtocol proto = frame.module.getUniverse().getProtocol(value.getTypeId());
    if (proto == null) {
      return value.getTypeId().toString();
    } else {
      return proto.getName();
    }
  }

  private StackTraceElement[] buildStackTrace() {
    ArrayList<StackTraceElement> trace = new ArrayList<StackTraceElement>();
    Frame current = frame;
    while (current.parent != null) {
      int opcode = current.code[current.pc];
      if (opcode == Opcode.kCall) {
        int index = current.code[current.pc + 1];
        int argc = current.code[current.pc + 2];
        Object name = current.getLiteral(index);
        RValue self = current.getArgument(argc, 0);
        String recvType = renderValueType(current, self);
        StringBuilder argTypes = new StringBuilder();
        if (argc > 1) {
          argTypes.append("[");
          for (int i = 1; i < argc; i++) {
            if (i > 1) argTypes.append(", ");
            RValue arg = current.getArgument(argc, i);
            argTypes.append(renderValueType(current, arg));
          }
          argTypes.append("]");
        }
        trace.add(new StackTraceElement(recvType, name.toString() + argTypes,
            current.bundle.fileName, -1));
      }
      current = current.parent;
    }
    return trace.toArray(new StackTraceElement[trace.size()]);
  }

  @Override
  public void printStackTrace(PrintStream out) {
    out.println();
    out.println(this.getClass().getName() + ": " + getMessage());
    try {
      for (StackTraceElement elm : getStackTrace()) {
        out.print("        at ");
        out.println(elm.toString());
      }
    } catch (Throwable t) {
      t.printStackTrace();
    }
  }

  public static class UndefinedGlobal extends InterpreterError {

    private final String name;

    public UndefinedGlobal(String name, Frame frame) {
      super(frame);
      this.name = name;
    }

    @Override
    public String getMessage() {
      return "Undefined global " + name;
    }

  }

  public static class MethodNotFound extends InterpreterError {

    public MethodNotFound(Frame frame) {
      super(frame);
    }

    @Override
    public String getMessage() {
      StackTraceElement topFrame = getStackTrace()[0];
      return "Method " + topFrame.getClassName() + "/"
          + topFrame.getMethodName() + " not found";
    }

  }

}
