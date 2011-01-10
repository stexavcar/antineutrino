package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.List;

import org.javatrino.ast.Symbol;
import org.neutrino.compiler.ResolverSymbol.LocalSymbol;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.syntax.Tree.LocalDefinition;

public abstract class Scope {

  public abstract ResolverSymbol lookupResolver(String name);

  public abstract Symbol lookup(String name);

  public static class CapturingScope extends Scope {

    private final List<ResolverSymbol> outerTransient = new ArrayList<ResolverSymbol>();
    private final List<ResolverSymbol> outerCaptures = new ArrayList<ResolverSymbol>();
    private final List<RFieldKey> outerCaptureFields = new ArrayList<RFieldKey>();
    private final Scope outerScope;

    public CapturingScope(Scope outer) {
      this.outerScope = outer;
    }

    public List<ResolverSymbol> getOuterTransient() {
      return outerTransient;
    }

    public List<RFieldKey> getOuterTransientFields() {
      return outerCaptureFields;
    }

    @Override
    public ResolverSymbol lookupResolver(String name) {
      ResolverSymbol outerResult = outerScope.lookupResolver(name);
      if (!outerResult.isTransient())
        return outerResult;
      int index = outerTransient.indexOf(outerResult);
      if (index == -1) {
        RFieldKey field = new RFieldKey();
        ResolverSymbol capture = ResolverSymbol.outer(outerResult, field);
        outerTransient.add(outerResult);
        outerCaptures.add(capture);
        outerCaptureFields.add(field);
        return capture;
      } else {
        if (index >= outerCaptures.size()) {
          throw new RuntimeException("Failed lookup for " + name);
        }
        return outerCaptures.get(index);
      }
    }

    @Override
    public Symbol lookup(String name) {
      return outerScope.lookup(name);
    }

  }

  public static class MethodScope extends Scope {

    private final List<Parameter> params;
    private final List<ResolverSymbol> symbols;
    private final Scope outerScope;
    private int localCount = 0;

    public MethodScope(List<Parameter> params, Scope outer) {
      this.params = params;
      this.outerScope = outer;
      this.symbols = new ArrayList<ResolverSymbol>();
      for (int i = 0; i < params.size(); i++)
        symbols.add(ResolverSymbol.parameter(i, params.size()));
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
    public ResolverSymbol lookupResolver(String name) {
      for (int i = 0; i < params.size(); i++) {
        if (params.get(i).getName().equals(name))
          return symbols.get(i);
      }
      return outerScope.lookupResolver(name);
    }

    @Override
    public Symbol lookup(String name) {
      for (Parameter param : params) {
        if (param.getName().equals(name))
          return param.getSymbol();
      }
      return outerScope.lookup(name);
    }

  }

  private static class GlobalScope extends Scope {

    private static final GlobalScope INSTANCE = new GlobalScope();

    @Override
    public ResolverSymbol lookupResolver(String name) {
      return ResolverSymbol.global(name);
    }

    @Override
    public Symbol lookup(String name) {
      return null;
    }

  }

  public static Scope globalScope() {
    return GlobalScope.INSTANCE;
  }

  public static class LocalScope extends Scope {

    private final Scope outer;
    private final String name;
    private final LocalSymbol resolverSymbol;
    private final Symbol symbol;

    private LocalScope(LocalDefinition def, int index, Scope outer) {
      this.outer = outer;
      this.name = def.getName();
      this.symbol = def.getSymbol();
      this.resolverSymbol = ResolverSymbol.local(def.isReference(), index);
    }

    public LocalSymbol getSymbol() {
      return resolverSymbol;
    }

    @Override
    public ResolverSymbol lookupResolver(String name) {
      if (name.equals(this.name))
        return resolverSymbol;
      return outer.lookupResolver(name);
    }

     @Override
    public Symbol lookup(String name) {
      if (name.equals(this.name))
        return this.symbol;
      return outer.lookup(name);
    }

  }

  public static LocalScope localScope(LocalDefinition def,
      int index, Scope outer) {
    return new LocalScope(def, index, outer);
  }

}
