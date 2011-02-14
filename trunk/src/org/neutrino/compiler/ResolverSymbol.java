package org.neutrino.compiler;

import org.neutrino.runtime.RFieldKey;
import org.neutrino.syntax.Tree.Declaration;

public abstract class ResolverSymbol {

  /**
   * Is this symbol transient, that is, if an inner scope accesses
   * this should it make a copy rather than rely on it continuing to
   * be available in the outside scope.
   */
  public abstract boolean isTransient();

  public boolean isReference() {
    return false;
  }

  private static class GlobalSymbol extends ResolverSymbol {

    private final String name;

    public GlobalSymbol(String name) {
      this.name = name;
    }

    @Override
    public Declaration getStaticValue(CompilerUniverse universe) {
      return universe.findDeclaration(name);
    }

    @Override
    public boolean isTransient() {
      return false;
    }

  }

  public static ResolverSymbol global(String name) {
    return new GlobalSymbol(name);
  }

  private static class ParameterSymbol extends ResolverSymbol {

    public ParameterSymbol(int index) {
    }

    @Override
    public boolean isTransient() {
      return true;
    }

  }

  public static ResolverSymbol parameter(int index, int argc) {
    return new ParameterSymbol(index);
  }

  private static class OuterSymbol extends ResolverSymbol {

    private final ResolverSymbol outer;

    public OuterSymbol(ResolverSymbol outer) {
      this.outer = outer;
    }

    @Override
    public boolean isTransient() {
      return true;
    }

    @Override
    public boolean isReference() {
      return outer.isReference();
    }

  }

  public static ResolverSymbol outer(ResolverSymbol outerSymbol, RFieldKey field) {
    return new OuterSymbol(outerSymbol);
  }

  public static class LocalSymbol extends ResolverSymbol {

    private final boolean isReference;
    private final int index;

    public LocalSymbol(boolean isReference, int index) {
      this.isReference = isReference;
      this.index = index;
    }

    public int getIndex() {
      return index;
    }

    @Override
    public boolean isTransient() {
      return true;
    }

    @Override
    public boolean isReference() {
      return isReference;
    }

  }

  public static LocalSymbol local(boolean isReference, int index) {
    return new LocalSymbol(isReference, index);
  }

  public Declaration getStaticValue(CompilerUniverse universe) {
    return null;
  }

}
