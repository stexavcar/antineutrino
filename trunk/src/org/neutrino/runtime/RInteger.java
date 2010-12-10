package org.neutrino.runtime;

import org.neutrino.oldton.ISeedable;
import org.neutrino.oldton.Oldton;
import org.neutrino.oldton.PInteger;
import org.neutrino.oldton.PMap;
import org.neutrino.oldton.PString;
import org.neutrino.oldton.PValue;
import org.neutrino.oldton.annotations.Factory;
import org.neutrino.oldton.annotations.Growable;
import org.neutrino.oldton.annotations.SeedMember;
import org.neutrino.plankton.Store;

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

  public @Store @SeedMember int value;

  private RInteger(int value) {
    this.value = value;
  }

  public RInteger() { }

  public static RInteger get(int value) {
    if (0 <= value && value < kCachedCount)
      return kCache[value];
    return new RInteger(value);
  }

  private static final PString VALUE = Oldton.newString("value");
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
