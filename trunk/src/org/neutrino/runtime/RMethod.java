package org.neutrino.runtime;

import java.util.List;

import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Parameter;
import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(RMethod.TAG)
public class RMethod extends RValue implements ISeedable {

  private static final String TAG = "org::neutrino::runtime::RMethod";

  public @SeedMember String name;
  public @SeedMember List<Parameter> params;
  public @SeedMember CodeBundle code;

  public RMethod(String name, List<Parameter> params, CodeBundle code) {
    this.name = name;
    this.params = params;
    this.code = code;
  }

  public RMethod() { }

}
