package org.neutrino.runtime;

import java.io.PrintStream;
import java.util.ArrayList;

import org.javatrino.bytecode.Opcode;
import org.neutrino.pib.Universe;

@SuppressWarnings("serial")
public class InterpreterError extends RuntimeException {

  private final Frame frame;
  private final Universe universe;
  private StackTraceElement[] trace;

  public InterpreterError(Throwable cause, Frame frame, Universe universe) {
    super(cause);
    this.frame = frame;
    this.universe = universe;
  }

  public InterpreterError(Frame frame, Universe universe) {
    this.frame = frame;
    this.universe = universe;
  }

  @Override
  public StackTraceElement[] getStackTrace() {
    if (trace == null)
      trace = buildStackTrace();
    return trace;
  }

  private String renderValueType(Frame frame, RValue value) {
    for (TypeId typeId : value.getTypeIds()) {
      RProtocol proto = universe.getProtocol(typeId);
      if (proto != null) {
        return proto.getName();
      }
    }
    return "?";
  }

  private StackTraceElement[] buildStackTrace() {
    ArrayList<StackTraceElement> trace = new ArrayList<StackTraceElement>();
    Frame current = frame;
    while (current.parent != null) {
      int opcode = current.bundle.getCode()[current.pc];
      if (opcode == Opcode.kCall) {
        int index = current.bundle.getCode()[current.pc + 1];
        int argc = current.bundle.getCode()[current.pc + 2];
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
    Throwable cause = getCause();
    if (cause != null) {
      out.println("caused by");
      cause.printStackTrace(out);
    }
  }

  public static class UndefinedGlobal extends InterpreterError {

    private final Object name;

    public UndefinedGlobal(Object name, Frame frame, Universe universe) {
      super(frame, universe);
      this.name = name;
    }

    @Override
    public String getMessage() {
      return "Undefined global " + name;
    }

  }

  public static class MethodNotFound extends InterpreterError {

    public MethodNotFound(Frame frame, Universe universe) {
      super(frame, universe);
    }

    @Override
    public String getMessage() {
      StackTraceElement topFrame = getStackTrace()[0];
      return "Method " + topFrame.getClassName() + "/"
          + topFrame.getMethodName() + " not found";
    }

  }

  public static class Failure extends InterpreterError {

    private final String message;

    public Failure(String message, Frame frame, Universe universe) {
      super(frame, universe);
      this.message = message;
    }

    @Override
    public String getMessage() {
      return message;
    }

  }

}
