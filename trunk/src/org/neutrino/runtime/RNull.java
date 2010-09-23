package org.neutrino.runtime;

public class RNull extends RValue {

  private static final RNull INSTANCE = new RNull();

  private RNull() { }

  public static RNull getInstance() {
    return INSTANCE;
  }

  public String toString() {
    return "#n";
  }

}
