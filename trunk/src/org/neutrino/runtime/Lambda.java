package org.neutrino.runtime;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;

public class Lambda extends RValue {

  private static final RProtocol[] TYPE_IDS = RProtocol.getCanonicals("Lambda");

  private final Module module;
  private final CodeBundle bundle;

  public Lambda(Module module, CodeBundle bundle) {
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
  public RProtocol[] getTypeIds() {
    return TYPE_IDS;
  }

  @Override
  public String toString() {
    return "#<a Lambda: " + bundle + ">";
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
