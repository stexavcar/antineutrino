package org.neutrino.runtime;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(RInteger.TAG)
public class RInteger extends RValue implements ISeedable {

  static final String TAG = "org::neutrino::runtime::RInteger";
  private static final TypeId TYPE_ID = TypeId.get("int");

  public @SeedMember int value;

  public RInteger(int value) {
    this.value = value;
  }

  public RInteger() { }

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

}
