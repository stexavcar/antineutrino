package org.neutrino.pib;

import org.javatrino.ast.Symbol;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.TypeId;

public class Parameter {

  public @Store String name;
  public @Store String type;
  public @Store boolean isProtocolMethod;
  public @Store Symbol symbol;
  private TypeId typeId;

  public Parameter(String name, String type, boolean isProtocolMethod) {
    this.name = name;
    this.type = type == null ? "Object" : type;
    this.isProtocolMethod = isProtocolMethod;
    this.symbol = new Symbol(Symbol.kParameterSymbol);
  }

  public Parameter() { }

  public TypeId ensureTypeId(Module module) {
    if (this.typeId == null) {
      if ("Object".equals(type))
        return null;
      RProtocol proto = module.getProtocol(type);
      if (proto == null) {
        module.getProtocol(type);
        assert false : "Undefined protocol " + type;
      }
      this.typeId = isProtocolMethod ? proto.getProtocolTypeId() : proto.getInstanceTypeId();
    }
    return this.typeId;
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

  public Symbol getSymbol() {
    return symbol;
  }

}
