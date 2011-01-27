package org.neutrino.compiler;

import org.neutrino.compiler.Scope.CapturingScope;
import org.neutrino.compiler.Scope.LocalScope;
import org.neutrino.compiler.Scope.MethodScope;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Assignment;
import org.neutrino.syntax.Tree.Identifier;
import org.neutrino.syntax.Tree.Internal;
import org.neutrino.syntax.Tree.LocalDefinition;
import org.neutrino.syntax.Tree.Method;
import org.neutrino.syntax.Tree.New;

public class LexicalAnalyzer extends Tree.ExpressionVisitor<Void> {

  private Scope scope = Scope.globalScope();
  private MethodScope methodScope = null;

  public LexicalAnalyzer() { }

  public LexicalAnalyzer(Method method) {
    methodScope = new Scope.MethodScope(method.getParameters(), scope);
    scope = methodScope;
  }

  public int getLocalCount() {
    return methodScope.getLocalCount();
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
  public Void visitInternal(Internal that) {
    int argc = methodScope.getParameters().size() + 1;
    that.setArgumentCount(argc);
    return null;
  }

  @Override
  public Void visitNew(New that) {
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
      CapturingScope capturingScope = new Scope.CapturingScope(prevScope);
      scope = capturingScope;
      for (New.Field field : that.getFields()) {
        if (!field.hasEagerValue()) {
          visitLazyField(field);
        }
      }
      that.setCaptures(capturingScope.getOuterTransient(), capturingScope.getOuterTransientFields());
    } finally {
      scope = prevScope;
    }
    return null;
  }

  @Override
  public Void visitIdentifier(Identifier that) {
    String name = that.getName();
    that.bind(scope.lookupResolver(name), scope.lookup(name));
    return null;
  }

  @Override
  public Void visitAssignment(Assignment that) {
    that.getValue().accept(this);
    String name = that.getName();
    ResolverSymbol symbol = scope.lookupResolver(name);
    assert symbol.isReference() : "Cannot assign to " + name;
    that.bind(symbol, scope.lookup(name));
    return null;
  }

  @Override
  public Void visitLocalDefinition(LocalDefinition that) {
    that.getValue().accept(this);
    Scope prevScope = scope;
    try {
      LocalScope local = Scope.localScope(that, methodScope.nextLocal(),
          prevScope);
      that.bind(local.getSymbol());
      scope = local;
      that.getBody().accept(this);
    } finally {
      scope = prevScope;
    }
    return null;
  }

}
