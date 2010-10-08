package org.neutrino.runtime;


public class LookupError extends InterpreterError {

  public LookupError(Frame frame) {
    super(frame);
  }

  @Override
  public String getMessage() {
    StackTraceElement topFrame = getStackTrace()[0];
    return "Method " + topFrame.getClassName() + "."
        + topFrame.getMethodName() + " not found";
  }

}
