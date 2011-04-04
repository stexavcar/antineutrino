package org.neutrino.pib;

import org.javatrino.ast.Symbol;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;

public class Parameter {

  public @Store String name;
  public @Store String type;
  public @Store boolean isProtocolMethod;
  public @Store Symbol symbol;
  private RProtocol protocol;

  public Parameter(String name, String type, boolean isProtocolMethod) {
    this.name = name;
    this.type = type == null ? "Object" : type;
    this.isProtocolMethod = isProtocolMethod;
    this.symbol = new Symbol(Symbol.kParameterSymbol);
  }

  public Parameter() { }

  public RProtocol ensureProtocol(Module module) {
    if (this.protocol == null) {
      if ("Object".equals(type))
        return null;
      this.protocol = (RProtocol) module.getGlobal(RString.of(type));
      if (this.protocol == null) {
        assert false : "Undefined protocol " + type;
      }
    }
    return this.protocol;
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
