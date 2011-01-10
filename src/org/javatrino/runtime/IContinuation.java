package org.javatrino.runtime;

import org.javatrino.value.Value;

public interface IContinuation {

  public void run(Value value);
  public boolean isDone();

}
