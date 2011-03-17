package org.javatrino.ast;

import org.neutrino.plankton.Store;
import org.neutrino.runtime.RProtocol;

public abstract class Test {

  public enum Type {
    EQ, IS, ANY
  }

  public abstract Type getType();

  public static class Eq extends Test {

    public @Store Object value;

    public Eq(Object value) {
      assert value != null;
      this.value = value;
    }

    public Eq() { }

    @Override
    public String toString() {
      return "eq(" + value + ")";
    }

    @Override
    public Type getType() {
      return Type.EQ;
    }

  }

  public static class Is extends Test {

    public @Store RProtocol type;

    public Is(RProtocol type) {
      assert type != null;
      this.type = type;
    }

    public Is() { }

    @Override
    public String toString() {
      return "is(" + type + ")";
    }

    @Override
    public Type getType() {
      return Type.IS;
    }

  }

  public static class Any extends Test {

    @Override
    public String toString() {
      return "_";
    }

    @Override
    public Type getType() {
      return Type.ANY;
    }

  }

}
