package org.neutrino.runtime;

import java.util.Arrays;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;

public class RLambda extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("Lambda");

  private final Module module;
  private final CodeBundle bundle;
  private final RValue[] captured;

  public RLambda(Module module, CodeBundle bundle, RValue[] captured) {
    this.module = module;
    this.bundle = bundle;
    this.captured = captured;
  }

  public Module getModule() {
    return this.module;
  }

  public RValue[] getOuter() {
    return this.captured;
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
    boolean hasCaptured = (captured != null) && captured.length > 0;
    String captureText = hasCaptured ? (" : " + Arrays.asList(captured)) : "";
    return "#<a Lambda: " + bundle + captureText + ">";
  }

}
