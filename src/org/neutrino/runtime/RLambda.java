package org.neutrino.runtime;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;

public class RLambda extends RValue {

  private final Module module;
  private final CodeBundle bundle;

  public RLambda(Module module, CodeBundle bundle) {
    this.module = module;
    this.bundle = bundle;
  }

  public Module getModule() {
    return this.module;
  }

  public CodeBundle getCode() {
    return this.bundle;
  }

  @Override
  public String toString() {
    return "#<a Lambda>";
  }

}
