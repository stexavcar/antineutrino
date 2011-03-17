package org.neutrino.compiler;

import org.neutrino.pib.Universe;
import org.neutrino.syntax.SyntaxError;
import org.neutrino.syntax.Tree.Declaration;
import org.neutrino.syntax.Tree.Method;


public class CompilerUniverse {

  private final CompilerModule root;

  public CompilerUniverse(CompilerModule root) {
    this.root = root;
  }

  public void parseAll() throws SyntaxError {
    root.parseAll();
  }

  public void writeTo(Universe universe) {
    root.writeStatics(universe);
    root.writeTo(universe);
  }

  public Declaration findDeclaration(String name) {
    return root.findDeclaration(name);
  }

  public Method findMethod(String holder, String name) {
    return root.findMethod(holder, name);
  }

}
