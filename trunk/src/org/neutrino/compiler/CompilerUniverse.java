package org.neutrino.compiler;

import org.neutrino.pib.Universe;
import org.neutrino.syntax.SyntaxError;
import org.neutrino.syntax.Tree.Declaration;


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

  public Declaration findDeclaration(String name) {
    return root.findDeclaration(name);
  }

}
