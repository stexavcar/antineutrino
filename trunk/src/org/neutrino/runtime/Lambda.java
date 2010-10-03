package org.neutrino.runtime;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;

public class Lambda extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("Lambda");

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
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return "#<a Lambda: " + bundle + ">";
  }

}
