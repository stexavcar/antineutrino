package org.neutrino.runtime;

import java.util.List;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(RProtocol.TAG)
public class RProtocol extends RValue implements ISeedable {

  private static final String TAG = "org::neutrino::runtime::RProtocol";

  public @SeedMember List<String> annotations;
  public @SeedMember String name;

  public RProtocol(List<String> annotations, String name) {
    this.annotations = annotations;
    this.name = name;
  }

  public RProtocol() { }

}
