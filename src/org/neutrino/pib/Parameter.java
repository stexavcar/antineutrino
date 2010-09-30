package org.neutrino.pib;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.TypeId;

@Growable(Parameter.TAG)
public class Parameter implements ISeedable {

  private static final String TAG = "org::neutrino::pib::Parameter";

  public @SeedMember String name;
  public @SeedMember String type;
  private TypeId typeId;

  public Parameter(String name, String type) {
    this.name = name;
    this.type = type;
  }

  public Parameter() { }

  public void initialize(Module module) {
    if ("Object".equals(type))
      return;
    RProtocol proto = module.getProtocol(type);
    assert proto != null : "Undefined protocol " + type;
    this.typeId = proto.getInstanceTypeId();
  }

  public TypeId getTypeId() {
    return this.typeId;
  }

  public String getName() {
    return this.name;
  }

  @Override
  public String toString() {
    if ("Object".equals(type)) return name;
    else return "(" + type + " " + name + ")";
  }

}
