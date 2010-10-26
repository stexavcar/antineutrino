package org.neutrino.compiler;

import org.neutrino.pib.BinaryBuilder;
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

  public void writeToBinary(BinaryBuilder builder) {
    root.writeToBinary(builder);
  }

  public Declaration findDeclaration(String name) {
    return root.findDeclaration(name);
  }

  public Method findMethod(String holder, String name) {
    return root.findMethod(holder, name);
  }

}
