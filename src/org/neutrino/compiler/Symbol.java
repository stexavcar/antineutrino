package org.neutrino.compiler;

import org.neutrino.pib.Assembler;

public abstract class Symbol {

  public abstract void emit(Assembler assm);

  /**
   * Is this symbol transient, that is, if an inner scope accesses
   * this should it make a copy rather than rely on it continuing to
   * be available in the outside scope.
   */
  public abstract boolean isTransient();

  private static class GlobalSymbol extends Symbol {

    private final String name;

    public GlobalSymbol(String name) {
      this.name = name;
    }

    @Override
    public void emit(Assembler assm) {
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
    public void emit(Assembler assm) {
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

    private final int index;

    public OuterSymbol(int index) {
      this.index = index;
    }

    @Override
    public void emit(Assembler assm) {
      assm.outer(index);
    }

    @Override
    public boolean isTransient() {
      return true;
    }

  }

  public static Symbol outer(int index) {
    return new OuterSymbol(index);
  }

}
