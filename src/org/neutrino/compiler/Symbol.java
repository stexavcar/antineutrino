package org.neutrino.compiler;

import org.neutrino.pib.Assembler;

public abstract class Symbol {

  public abstract void emit(Assembler assm);

  private static class GlobalSymbol extends Symbol {

    private final String name;

    public GlobalSymbol(String name) {
      this.name = name;
    }

    @Override
    public void emit(Assembler assm) {
      assm.global(name);
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

  }

  public static Symbol parameter(int index, int argc) {
    return new ParameterSymbol(index, argc);
  }

}
