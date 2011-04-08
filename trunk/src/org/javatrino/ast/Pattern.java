package org.javatrino.ast;

import java.util.List;

import org.neutrino.plankton.Store;
import org.neutrino.runtime.RValue;

public class Pattern {

  public @Store List<RValue> tags;
  public @Store Test test;
  public @Store Symbol symbol;

  public Pattern(List<RValue> tags, Test test, Symbol symbol) {
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
