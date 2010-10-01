package org.neutrino.compiler;

import org.neutrino.compiler.Scope.LocalScope;
import org.neutrino.compiler.Scope.MethodScope;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Identifier;
import org.neutrino.syntax.Tree.Lambda;
import org.neutrino.syntax.Tree.LocalDefinition;

public class LexicalAnalyzer extends Tree.ExpressionVisitor {

  private Scope scope = Scope.globalScope();
  private MethodScope methodScope = null;

  @Override
  public void visitLambda(Lambda that) {
    Scope prevScope = scope;
    MethodScope prevMethodScope = methodScope;
    try {
      methodScope = new MethodScope(that.getParameters(), prevScope);
      scope = methodScope;
      that.getBody().accept(this);
      that.bind(methodScope);
    } finally {
      scope = prevScope;
      methodScope = prevMethodScope;
    }
  }

  @Override
  public void visitIdentifier(Identifier that) {
    String name = that.getName();
    that.bind(scope.lookup(name));
  }

  @Override
  public void visitLocalDefinition(LocalDefinition that) {
    that.getValue().accept(this);
    Scope prevScope = scope;
    try {
      LocalScope local = Scope.localScope(that.getName(), methodScope.nextLocal(), prevScope);
      that.bind(local.getSymbol());
      scope = local;
      that.getBody().accept(this);
    } finally {
      scope = prevScope;
    }
  }

}
