package org.neutrino.compiler;

import org.neutrino.pib.Assembler;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.syntax.Tree.Declaration;

public abstract class ResolverSymbol {

  public abstract int emitLoad(Assembler assm);

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
    public int emitLoad(Assembler assm) {
      return assm.global(name);
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

    private final int index;
    private final int argc;

    public ParameterSymbol(int index, int argc) {
      this.index = index;
      this.argc = argc;
    }

    @Override
    public int emitLoad(Assembler assm) {
      return assm.argument(argc - index);
    }

    @Override
    public boolean isTransient() {
      return true;
    }

  }

  public static ResolverSymbol parameter(int index, int argc) {
    return new ParameterSymbol(index, argc);
  }

  private static class OuterSymbol extends ResolverSymbol {

    private final ResolverSymbol outer;
    private final RFieldKey field;

    public OuterSymbol(ResolverSymbol outer, RFieldKey field) {
      this.outer = outer;
      this.field = field;
    }

    @Override
    public int emitLoad(Assembler assm) {
      return assm.outer(field);
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
    return new OuterSymbol(outerSymbol, field);
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
    public int emitLoad(Assembler assm) {
      return assm.local(index);
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
