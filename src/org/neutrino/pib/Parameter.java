package org.neutrino.pib;

import org.neutrino.plankton.Store;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.TypeId;

public class Parameter {

  public @Store String name;
  public @Store String type;
  public @Store boolean isProtocolMethod;
  private TypeId typeId;

  public Parameter(String name, String type, boolean isProtocolMethod) {
    this.name = name;
    this.type = type == null ? "Object" : type;
    this.isProtocolMethod = isProtocolMethod;
  }

  public Parameter() { }

  public void initialize(Module module) {
    if ("Object".equals(type))
      return;
    RProtocol proto = module.getProtocol(type);
    assert proto != null : "Undefined protocol " + type;
    this.typeId = isProtocolMethod ? proto.getTypeId() : proto.getInstanceTypeId();
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
