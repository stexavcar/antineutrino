package org.neutrino.pib;

import java.util.Collections;
import java.util.List;

import org.javatrino.ast.Symbol;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;

public class Parameter {

  public @Store String name;
  public @Store List<RValue> keywords;
  public @Store String type;
  public @Store boolean isProtocolMethod;
  public @Store Symbol symbol;
  private RProtocol protocol;

  public Parameter(String name, List<RValue> keywords, String type, boolean isProtocolMethod) {
    this.name = name;
    this.keywords = keywords;
    this.type = type == null ? "Object" : type;
    this.isProtocolMethod = isProtocolMethod;
    this.symbol = new Symbol(Symbol.kParameterSymbol);
  }

  public Parameter(String name, RValue keyword, String type, boolean isProtocolMethod) {
    this(name, Collections.singletonList(keyword), type, isProtocolMethod);
  }

  public Parameter() { }

  public List<RValue> getKeywords() {
    return keywords;
  }

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
