package org.neutrino.runtime;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.syntax.Annotation;

import java.util.List;

@Growable(RProtocol.TAG)
public class RProtocol extends RValue implements ISeedable {

  private static final TypeId TYPE_ID = new TypeId();
  private static final String TAG = "org::neutrino::runtime::RProtocol";

  public @SeedMember List<Annotation> annotations;
  public @SeedMember String name;

  public RProtocol(List<Annotation> annotations, String name) {
    this.annotations = annotations;
    this.name = name;
  }

  public RProtocol() { }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return "#<a Protocol: " + name + ">";
  }

}
