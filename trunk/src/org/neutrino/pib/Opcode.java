package org.neutrino.pib;

/**
 * Opcodes used to encode syntax trees in platform independent binaries.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Opcode {

  public static final int kNumber = 0;
  public static final int kLambda = 1;
  public static final int kSlap = 2;
  public static final int kCall = 3;
  public static final int kReturn = 4;
  public static final int kNull = 5;
  public static final int kTrue = 6;
  public static final int kPush = 7;
  public static final int kCallNative = 8;

}
