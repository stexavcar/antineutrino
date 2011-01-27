package org.javatrino.ast;

import java.util.List;

import org.neutrino.plankton.Store;

public class Pattern {

  public @Store List<Object> tags;
  public @Store Test test;
  public @Store Symbol symbol;

  public Pattern(List<Object> tags, Test test, Symbol symbol) {
    this.tags = tags;
    this.test = test;
    this.symbol = symbol;
  }

  public Pattern() { }

  @Override
  public String toString() {
    return "pattern(" + tags + ": " + test + ")";
  }

}
