package org.neutrino.compiler;

import org.neutrino.compiler.Scope.MethodScope;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Identifier;
import org.neutrino.syntax.Tree.Lambda;

public class LexicalAnalyzer extends Tree.ExpressionVisitor {

  private Scope scope = Scope.globalScope();

  @Override
  public void visitLambda(Lambda that) {
    Scope prev = scope;
    try {
      MethodScope scope = new MethodScope(that.getParameters(), prev);
      this.scope = scope;
      that.getBody().accept(this);
      that.bind(scope.getOuterTransient());
    } finally {
      this.scope = prev;
    }
  }

  @Override
  public void visitIdentifier(Identifier that) {
    String name = that.getName();
    that.bind(scope.lookup(name));
  }

}
