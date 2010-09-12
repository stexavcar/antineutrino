package org.neutrino.runtime;

public class RInteger extends RValue {

  private final int value;

  public RInteger(int value) {
    this.value = value;
  }

  @Override
  public String toString() {
    return Integer.toString(value);
  }

}
