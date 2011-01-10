package org.javatrino.ast;

import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RValue;

public class Test {

  public static class Eq extends Test {

    private final RValue value;

    public Eq(RValue value) {
      this.value = value;
    }

  }

  public static class Is extends Test {

    private final RProtocol proto;

    public Is(RProtocol proto) {
      this.proto = proto;
    }

  }

  public static class Any extends Test {

  }

}
