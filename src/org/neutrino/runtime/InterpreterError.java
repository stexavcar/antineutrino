package org.neutrino.runtime;

import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;

import org.javatrino.bytecode.Opcode;
import org.javatrino.utils.Utils;
import org.neutrino.runtime.lookup.CallInfo;
import org.neutrino.runtime.lookup.CallInfo.ArgumentEntry;

@SuppressWarnings("serial")
public class InterpreterError extends RuntimeException {

  private final Frame frame;
  private StackTraceElement[] trace;

  public InterpreterError(Throwable cause, Frame frame) {
    super(cause);
    this.frame = frame;
  }

  public InterpreterError(Frame frame) {
    this.frame = frame;
  }

  @Override
  public StackTraceElement[] getStackTrace() {
    if (trace == null)
      trace = buildStackTrace();
    return trace;
  }

  private String renderValueType(RValue value) {
    return (value == null) ? "(null)" : value.toExternalString();
  }

  private static String[] getBrackets(String methodName) {
    String start, end;
    if (methodName.equals("()")) {
      start = "(";
      end = ")";
    } else if (methodName.equals("[]")) {
      start = "[";
      end = "]";
    } else {
      start = methodName + "(";
      end = ")";
    }
    return new String[] { start, end };
  }

  private StackTraceElement[] buildStackTrace() {
    ArrayList<StackTraceElement> trace = new ArrayList<StackTraceElement>();
    Frame current = frame;
    while (current.parent != null) {
      int opcode = current.bundle.getCode()[current.pc];
      if (opcode == Opcode.kCall) {
        int index = current.bundle.getCode()[current.pc + 1];
        int argc = current.bundle.getCode()[current.pc + 2];
        CallInfo info = (CallInfo) current.getLiteral(index);
        RValue self = getArgument(info, current, argc, RInteger.get(0));
        String selfStr = renderValueType(self);
        String name = renderArgument(info, current, argc, RString.of("name"));
        String[] bracks = getBrackets(name);
        List<String> args = new ArrayList<String>();
        for (int i = 1; i < argc - 1; i++) {
          String arg = renderArgument(info, current, argc, RInteger.get(i));
          args.add(arg);
        }
        String argsStr = Utils.join(args, ", ");
        trace.add(new StackTraceElement(selfStr, bracks[0] + argsStr + bracks[1],
            current.bundle.fileName, -1));
      }
      current = current.parent;
    }
    return trace.toArray(new StackTraceElement[trace.size()]);
  }

  private RValue getArgument(CallInfo info, Frame frame, int argc, RValue tag) {
    for (ArgumentEntry entry : info.entries) {
      if (entry.tag.equals(tag)) {
        return frame.getArgument(argc, entry.index);
      }
    }
    return null;
  }


  private String renderArgument(CallInfo info, Frame frame, int argc, RValue tag) {
    return renderValueType(getArgument(info, frame, argc, tag));
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

    public UndefinedGlobal(Object name, Frame frame) {
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
      return "Method " + topFrame.getClassName() + "."
          + topFrame.getMethodName() + " not found";
    }

  }

  public static class Failure extends InterpreterError {

    private final String message;

    public Failure(String message, Frame frame) {
      super(frame);
      this.message = message;
    }

    @Override
    public String getMessage() {
      return message;
    }

  }

}
