package org.neutrino.runtime;

public class RLambda extends RValue {

  private final int pc;
  private final byte[] code;

  public RLambda(int pc, byte[] code) {
    this.pc = pc;
    this.code = code;
  }

  @Override
  public String toString() {
    return "#<a Lambda>";
  }

}
