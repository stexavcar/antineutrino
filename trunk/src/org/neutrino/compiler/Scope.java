package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.List;

import org.neutrino.pib.Parameter;

public abstract class Scope {

  public abstract Symbol lookup(String name);

  private static class MethodScope extends Scope {

    private final List<Parameter> params;
    private final List<Symbol> symbols;
    private final Scope outer;

    public MethodScope(List<Parameter> params, Scope outer) {
      this.params = params;
      this.outer = outer;
      this.symbols = new ArrayList<Symbol>();
      for (int i = 0; i < params.size(); i++)
        symbols.add(Symbol.parameter(i, params.size()));
    }

    @Override
    public Symbol lookup(String name) {
      for (int i = 0; i < params.size(); i++) {
        if (params.get(i).getName().equals(name))
          return symbols.get(i);
      }
      return outer.lookup(name);
    }

  }

  public static Scope methodScope(List<Parameter> params, Scope outer) {
    return new MethodScope(params, outer);
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
