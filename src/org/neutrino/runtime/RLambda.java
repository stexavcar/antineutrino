package org.neutrino.runtime;

import org.neutrino.pib.CodeBundle;

public class RLambda extends RValue {

  private final CodeBundle bundle;

  public RLambda(CodeBundle bundle) {
    this.bundle = bundle;
  }

  @Override
  public String toString() {
    return "#<a Lambda>";
  }

}
