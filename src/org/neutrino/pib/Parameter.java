package org.neutrino.pib;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(Parameter.TAG)
public class Parameter implements ISeedable {

  private static final String TAG = "org::neutrino::pib::Parameter";

  public @SeedMember String name;
  public @SeedMember String type;

  public Parameter(String name, String type) {
    this.name = name;
    this.type = type;
  }

  public Parameter() { }

  @Override
  public String toString() {
    if ("Object".equals(type)) return name;
    else return "(" + type + " " + name + ")";
  }

}
