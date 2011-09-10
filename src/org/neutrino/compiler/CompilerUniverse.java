package org.neutrino.compiler;

import org.neutrino.pib.Universe;
import org.neutrino.syntax.SyntaxError;


public class CompilerUniverse {

  private final CompilerModule root;

  public CompilerUniverse(CompilerModule root) {
    this.root = root;
  }

  public void parseAll() throws SyntaxError {
    root.parseAll();
  }

  public void writeTo(Universe universe) {
    root.buildSkeleton(universe);
    root.writeTo(universe);
  }

  public void preCompileModules() {

  }

}
