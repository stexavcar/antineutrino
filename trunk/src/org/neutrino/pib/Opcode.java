package org.neutrino.pib;

/**
 * Opcodes used to encode syntax trees in platform independent binaries.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public enum Opcode {

  NUMBER(0),
  GLOBAL(1);

  public static final int kNumber = 0;

  private final int code;

  private Opcode(int code) {
    this.code = code;
  }

  public int getCode() {
    return this.code;
  }

}
