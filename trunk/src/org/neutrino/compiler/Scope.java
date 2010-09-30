package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.List;

import org.neutrino.pib.Parameter;

public abstract class Scope {

  public abstract Symbol lookup(String name);

  public static class MethodScope extends Scope {

    private final List<Parameter> params;
    private final List<Symbol> symbols;
    private final List<Symbol> outerTransient = new ArrayList<Symbol>();
    private final List<Symbol> outerCaptures = new ArrayList<Symbol>();
    private final Scope outerScope;

    public MethodScope(List<Parameter> params, Scope outer) {
      this.params = params;
      this.outerScope = outer;
      this.symbols = new ArrayList<Symbol>();
      for (int i = 0; i < params.size(); i++)
        symbols.add(Symbol.parameter(i, params.size()));
    }

    public List<Symbol> getOuterTransient() {
      return this.outerTransient;
    }

    @Override
    public Symbol lookup(String name) {
      for (int i = 0; i < params.size(); i++) {
        if (params.get(i).getName().equals(name))
          return symbols.get(i);
      }
      Symbol outerResult = outerScope.lookup(name);
      if (!outerResult.isTransient())
        return outerResult;
      int index = outerTransient.indexOf(outerResult);
      if (index == -1) {
        index = outerTransient.size();
        Symbol capture = Symbol.outer(index);
        outerTransient.add(outerResult);
        outerCaptures.add(capture);
      }
      return outerCaptures.get(index);
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

}
