package org.javatrino.ast;

import org.neutrino.plankton.Store;


public class Symbol {

  public static final int kParameterSymbol = 0;
  public static final int kLocalSymbol = 1;
  public static final int kImplicitThis = 2;

  public @Store int type;
  private int id = -1;

  public Symbol(int type) {
    this.type = type;
  }

  public Symbol() { }

  @Override
  public String toString() {
    return "#" + getId();
  }

  private int getId() {
    if (this.id == -1)
      this.id = nextId();
    return this.id;
  }

  private static int nextId = 0;
  private static synchronized int nextId() {
    return nextId++;
  }

}
