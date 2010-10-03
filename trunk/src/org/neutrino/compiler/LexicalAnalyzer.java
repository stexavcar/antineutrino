package org.neutrino.compiler;

import org.neutrino.compiler.Scope.CapturingScope;
import org.neutrino.compiler.Scope.LocalScope;
import org.neutrino.compiler.Scope.MethodScope;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Identifier;
import org.neutrino.syntax.Tree.Internal;
import org.neutrino.syntax.Tree.LocalDefinition;
import org.neutrino.syntax.Tree.Method;
import org.neutrino.syntax.Tree.New;

public class LexicalAnalyzer extends Tree.ExpressionVisitor {

  private Scope scope = Scope.globalScope();
  private MethodScope methodScope = null;

  public LexicalAnalyzer() { }

  public LexicalAnalyzer(Method method) {
    methodScope = new Scope.MethodScope(method.getParameters(), scope);
    scope = methodScope;
  }

  private void visitLazyField(New.Field field) {
    MethodScope prevMethodScope = methodScope;
    Scope prevScope = scope;
    try {
      scope = methodScope = new Scope.MethodScope(field.getParameters(), prevScope);
      field.getBody().accept(this);
      field.setLocalCount(methodScope.getLocalCount());
    } finally {
      scope = prevScope;
      methodScope = prevMethodScope;
    }
  }

  @Override
  public void visitInternal(Internal that) {
    int argc = methodScope.getParameters().size() + 1;
    that.setArgumentCount(argc);
  }

  @Override
  public void visitNew(New that) {
    // First visit eager fields within the current scope.
    int eagerFieldCount = 0;
    for (New.Field field : that.getFields()) {
      if (field.hasEagerValue()) {
        field.getBody().accept(this);
        eagerFieldCount++;
      }
    }
    // Then visit lazy fields within a capturing scope that captures
    // variables into the object.
    Scope prevScope = scope;
    try {
      CapturingScope capturingScope = new Scope.CapturingScope(prevScope,
          eagerFieldCount);
      scope = capturingScope;
      for (New.Field field : that.getFields()) {
        if (!field.hasEagerValue()) {
          visitLazyField(field);
        }
      }
      that.setCaptures(capturingScope.getOuterTransient());
    } finally {
      scope = prevScope;
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
