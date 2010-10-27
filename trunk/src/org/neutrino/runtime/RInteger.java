package org.neutrino.runtime;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.PInteger;
import org.neutrino.plankton.PMap;
import org.neutrino.plankton.PString;
import org.neutrino.plankton.PValue;
import org.neutrino.plankton.Plankton;
import org.neutrino.plankton.annotations.Factory;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(RInteger.TAG)
public class RInteger extends RValue implements ISeedable {

  static final String TAG = "org::neutrino::runtime::RInteger";
  private static final TypeId TYPE_ID = TypeId.get("int");

  private static final int kCachedCount = 1024;
  private static final RInteger[] kCache = new RInteger[kCachedCount];
  static {
    for (int i = 0 ; i < kCachedCount; i++)
      kCache[i] = new RInteger(i);
  }

  public final @SeedMember int value;

  private RInteger(int value) {
    this.value = value;
  }

  public static RInteger get(int value) {
    if (0 <= value && value < kCachedCount)
      return kCache[value];
    return new RInteger(value);
  }

  private static final PString VALUE = Plankton.newString("value");
  @Factory(initialize = false)
  public static RInteger grow(PValue value) {
    return get(((PInteger) ((PMap) value).get(VALUE, null)).getValue());
  }

  public int getValue() {
    return value;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
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

}
