package org.neutrino.runtime;

import org.neutrino.plankton.Store;
public class RInteger extends RValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("Integer");

  private static final int kCachedCount = 1024;
  private static final RInteger[] kCache = new RInteger[kCachedCount];
  static {
    for (int i = 0 ; i < kCachedCount; i++)
      kCache[i] = new RInteger(i);
  }

  public @Store int value;

  private RInteger(int value) {
    this.value = value;
  }

  public RInteger() { }

  public static RInteger get(int value) {
    if (0 <= value && value < kCachedCount)
      return kCache[value];
    return new RInteger(value);
  }

  public int getValue() {
    return value;
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  @Override
  public String toString() {
    return Integer.toString(value);
  }

  @Override
  public int hashCode() {
    return value;
  }

  @Override
  public boolean equals(Object obj) {
    return (obj instanceof RInteger) && ((RInteger) obj).value == this.value;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

  @Override
  public Kind getKind() {
    return Kind.INTEGER;
  }

  @Override
  public int compareTo(RValue that) {
    if (that instanceof RInteger) {
      return this.value - ((RInteger) that).value;
    } else {
      return this.getKind().compareTo(that.getKind());
    }
  }

}
