package org.neutrino.runtime;

import java.util.List;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Parameter;
import org.neutrino.plankton.Store;
import org.neutrino.syntax.Annotation;

public class RMethod extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("Method");

  public @Store List<Annotation> annots;
  public @Store String name;
  public @Store List<Parameter> params;
  public @Store CodeBundle code;
  public Module origin;

  public RMethod(List<Annotation> annots, String name, List<Parameter> params,
      CodeBundle code) {
    this.annots = annots;
    this.name = name;
    this.params = params;
    this.code = code;
  }

  public RMethod() { }

  public void initialize(Module module) {
    for (Parameter param : params)
      param.initialize(module);
    this.origin = module;
  }

  public String getName() {
    return this.name;
  }

  public CodeBundle getCode() {
    return this.code;
  }

  public List<Parameter> getParameters() {
    return this.params;
  }

  public Module getOrigin() {
    return origin;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return "#<a Method(" + name + "): " + code + ">";
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
