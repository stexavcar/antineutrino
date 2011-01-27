package org.javatrino.ast;

import java.util.ArrayList;
import java.util.List;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.plankton.Store;

public class Method {

  public @Store List<Pattern> signature;
  public @Store boolean acceptsExtraArguments;
  public @Store Expression body;
  public @Store Module origin;
  private CodeBundle bundle;

  public Method(List<Pattern> signature, boolean acceptsExtraArguments, Expression body) {
    this.signature = signature;
    this.acceptsExtraArguments = acceptsExtraArguments;
    this.body = body;
  }

  public Method() { }

  @Override
  public String toString() {
    return "method(" + signature.toString() + ")";
  }

  public CodeBundle getBundle() {
    if (bundle == null) {
      List<Symbol> syms = new ArrayList<Symbol>();
      for (Pattern pattern : signature) {
        if (pattern != null)
          syms.add(pattern.symbol);
      }
      bundle = new CodeBundle(null, null, -1, null, -1, body, syms);
    }
    return bundle;
  }

}
