package org.neutrino.compiler;

import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Identifier;
import org.neutrino.syntax.Tree.Lambda;

public class LexicalAnalyzer extends Tree.ExpressionVisitor {

  private Scope scope = Scope.globalScope();

  @Override
  public void visitLambda(Lambda that) {
    Scope prev = scope;
    try {
      scope = Scope.methodScope(that.getParameters(), prev);
      that.getBody().accept(this);
    } finally {
      scope = prev;
    }
  }

  @Override
  public void visitIdentifier(Identifier that) {
    String name = that.getName();
    that.bind(scope.lookup(name));
  }

}
