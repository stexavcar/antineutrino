package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.List;

import org.javatrino.ast.Symbol;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.syntax.Tree.LocalDefinition;

public abstract class Scope {

  public static class LookupResult {
    public final Symbol symbol;
    public final boolean isReference;
    public LookupResult(Symbol symbol, boolean isReference) {
      this.symbol = symbol;
      this.isReference = isReference;
    }
  }

  public abstract LookupResult lookup(String name);

  public static class CapturingScope extends Scope {

    private final List<RFieldKey> outerCaptureFields = new ArrayList<RFieldKey>();
    private final Scope outerScope;

    public CapturingScope(Scope outer) {
      this.outerScope = outer;
    }

    public List<RFieldKey> getOuterTransientFields() {
      return outerCaptureFields;
    }

    @Override
    public LookupResult lookup(String name) {
      return outerScope.lookup(name);
    }

  }

  public static class MethodScope extends Scope {

    private final List<Parameter> params;
    private final Scope outerScope;
    private int localCount = 0;

    public MethodScope(List<Parameter> params, Scope outer) {
      this.params = params;
      this.outerScope = outer;
    }

    public List<Parameter> getParameters() {
      return params;
    }

    public int nextLocal() {
      return localCount++;
    }

    public int getLocalCount() {
      return this.localCount;
    }

    @Override
    public LookupResult lookup(String name) {
      for (Parameter param : params) {
        if (param.getName().equals(name))
          return new LookupResult(param.getSymbol(), false);
      }
      return outerScope.lookup(name);
    }

  }

  private static class GlobalScope extends Scope {

    private static final GlobalScope INSTANCE = new GlobalScope();

    @Override
    public LookupResult lookup(String name) {
      return null;
    }

  }

  public static Scope globalScope() {
    return GlobalScope.INSTANCE;
  }

  public static class LocalScope extends Scope {

    private final Scope outer;
    private final String name;
    private final boolean isReference;
    private final Symbol symbol;

    private LocalScope(LocalDefinition def, int index, Scope outer) {
      this.outer = outer;
      this.name = def.getName();
      this.symbol = def.getSymbol();
      this.isReference = def.isReference();
    }

    @Override
    public LookupResult lookup(String name) {
      if (name.equals(this.name))
        return new LookupResult(this.symbol, isReference);
      return outer.lookup(name);
    }

  }

  public static LocalScope localScope(LocalDefinition def,
      int index, Scope outer) {
    return new LocalScope(def, index, outer);
  }

}
