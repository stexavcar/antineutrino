package org.javatrino.ast;

import org.neutrino.plankton.Store;


public class Symbol {

  public static final int kParameterSymbol = 0;
  public static final int kLocalSymbol = 1;

  public @Store int type;

  public Symbol(int type) {
    this.type = type;
  }

  public Symbol() { }

}
