package org.neutrino.runtime;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(RString.TAG)
public class RString extends RValue implements ISeedable {

  private static final TypeId TYPE_ID = TypeId.get("str");

  static final String TAG = "org::neutrino::runtime::RString";
  public @SeedMember String value;

  public RString(String value) {
    this.value = value;
  }

  public RString() { }

  public String getValue() {
    return value;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return "#'" + value + "'";
  }

  @Override
  public String toExternalString() {
    return value;
  }

  @Override
  public int hashCode() {
    return value.hashCode();
  }

  @Override
  public boolean equals(Object obj) {
    return (obj instanceof RString) && ((RString) obj).value.equals(this.value);
  }

}
