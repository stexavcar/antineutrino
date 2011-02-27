package org.neutrino.runtime;

import java.util.List;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Parameter;
import org.neutrino.plankton.Store;
import org.neutrino.syntax.Annotation;

public class RMethod extends RValue {

  private static final TypeId[] TYPE_IDS = TypeId.getIds("Method");

  public @Store List<Annotation> annots;
  public @Store String name;
  public @Store List<Parameter> params;
  public @Store CodeBundle code;
  public @Store Module module;

  public RMethod(Module module, List<Annotation> annots, String name, List<Parameter> params,
      CodeBundle code) {
    this.module = module;
    this.annots = annots;
    this.name = name;
    this.params = params;
    this.code = code;
  }

  public RMethod() { }

  public void initialize(Module module) {
    for (Parameter param : params)
      param.initialize(module);
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
    return module;
  }

  @Override
  public TypeId[] getTypeIds() {
    return TYPE_IDS;
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
