package org.neutrino.runtime;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Parameter;
import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.syntax.Annotation;

import java.util.List;

@Growable(RMethod.TAG)
public class RMethod extends RValue implements ISeedable {

  private static final TypeId TYPE_ID = TypeId.get("Method");

  private static final String TAG = "org::neutrino::runtime::RMethod";

  public @SeedMember List<Annotation> annots;
  public @SeedMember String name;
  public @SeedMember List<Parameter> params;
  public @SeedMember CodeBundle code;



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

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return "#<a Method(" + name + "): " + code + ">";
  }

}
