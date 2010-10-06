package org.neutrino.compiler;

import org.neutrino.pib.Assembler;

public abstract class Symbol {

  public abstract void emitLoad(Assembler assm);

  /**
   * Is this symbol transient, that is, if an inner scope accesses
   * this should it make a copy rather than rely on it continuing to
   * be available in the outside scope.
   */
  public abstract boolean isTransient();

  public boolean isReference() {
    return false;
  }

  private static class GlobalSymbol extends Symbol {

    private final String name;

    public GlobalSymbol(String name) {
      this.name = name;
    }

    @Override
    public void emitLoad(Assembler assm) {
      assm.global(name);
    }

    @Override
    public boolean isTransient() {
      return false;
    }

  }

  public static Symbol global(String name) {
    return new GlobalSymbol(name);
  }

  private static class ParameterSymbol extends Symbol {

    private final int index;
    private final int argc;

    public ParameterSymbol(int index, int argc) {
      this.index = index;
      this.argc = argc;
    }

    @Override
    public void emitLoad(Assembler assm) {
      assm.argument(argc - index);
    }

    @Override
    public boolean isTransient() {
      return true;
    }

  }

  public static Symbol parameter(int index, int argc) {
    return new ParameterSymbol(index, argc);
  }

  private static class OuterSymbol extends Symbol {

    private final Symbol outer;
    private final int index;

    public OuterSymbol(Symbol outer, int index) {
      this.outer = outer;
      this.index = index;
    }

    @Override
    public void emitLoad(Assembler assm) {
      assm.outer(index);
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

  public static Symbol outer(Symbol outerSymbol, int index) {
    return new OuterSymbol(outerSymbol, index);
  }

  public static class LocalSymbol extends Symbol {

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
    public void emitLoad(Assembler assm) {
      assm.local(index);
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

}
