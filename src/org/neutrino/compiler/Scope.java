package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.List;

import org.neutrino.compiler.Symbol.LocalSymbol;
import org.neutrino.pib.Parameter;
import org.neutrino.syntax.Tree.LocalDefinition;

public abstract class Scope {

  public abstract Symbol lookup(String name);

  public static class CapturingScope extends Scope {

    private final int symbolIndexOffset;
    private final List<Symbol> outerTransient = new ArrayList<Symbol>();
    private final List<Symbol> outerCaptures = new ArrayList<Symbol>();
    private final Scope outerScope;

    public CapturingScope(Scope outer, int symbolIndexOffset) {
      this.outerScope = outer;
      this.symbolIndexOffset = symbolIndexOffset;
    }

    public List<Symbol> getOuterTransient() {
      return outerTransient;
    }

    @Override
    public Symbol lookup(String name) {
      Symbol outerResult = outerScope.lookup(name);
      if (!outerResult.isTransient())
        return outerResult;
      int index = outerTransient.indexOf(outerResult);
      if (index == -1) {
        index = symbolIndexOffset + outerTransient.size();
        Symbol capture = Symbol.outer(outerResult, index);
        outerTransient.add(outerResult);
        outerCaptures.add(capture);
      }
      return outerCaptures.get(index);
    }

  }

  public static class MethodScope extends Scope {

    private final List<Parameter> params;
    private final List<Symbol> symbols;
    private final Scope outerScope;
    private int localCount = 0;

    public MethodScope(List<Parameter> params, Scope outer) {
      this.params = params;
      this.outerScope = outer;
      this.symbols = new ArrayList<Symbol>();
      for (int i = 0; i < params.size(); i++)
        symbols.add(Symbol.parameter(i, params.size()));
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
    public Symbol lookup(String name) {
      for (int i = 0; i < params.size(); i++) {
        if (params.get(i).getName().equals(name))
          return symbols.get(i);
      }
      return outerScope.lookup(name);
    }

  }

  private static class GlobalScope extends Scope {

    private static final GlobalScope INSTANCE = new GlobalScope();

    @Override
    public Symbol lookup(String name) {
      return Symbol.global(name);
    }

  }

  public static Scope globalScope() {
    return GlobalScope.INSTANCE;
  }

  public static class LocalScope extends Scope {

    private final Scope outer;
    private final String name;
    private final LocalSymbol symbol;

    private LocalScope(LocalDefinition def, int index, Scope outer) {
      this.outer = outer;
      this.name = def.getName();
      this.symbol = Symbol.local(def.isReference(), index);
    }

    public LocalSymbol getSymbol() {
      return symbol;
    }

    @Override
    public Symbol lookup(String name) {
      if (name.equals(this.name))
        return symbol;
      return outer.lookup(name);
    }

  }

  public static LocalScope localScope(LocalDefinition def,
      int index, Scope outer) {
    return new LocalScope(def, index, outer);
  }

}
