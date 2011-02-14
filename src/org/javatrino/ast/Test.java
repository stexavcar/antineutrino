package org.javatrino.ast;

import org.neutrino.plankton.Store;
import org.neutrino.runtime.TypeId;

public class Test {

  public static class Eq extends Test {

    public @Store Object value;

    public Eq(Object value) {
      this.value = value;
    }

    public Eq() { }

    @Override
    public String toString() {
      return "eq(" + value + ")";
    }

  }

  public static class Is extends Test {

    public @Store TypeId type;

    public Is(TypeId type) {
      this.type = type;
    }

    public Is() { }

    @Override
    public String toString() {
      return "is(" + type + ")";
    }

  }

  public static class Any extends Test {

    @Override
    public String toString() {
      return "_";
    }

  }

}
