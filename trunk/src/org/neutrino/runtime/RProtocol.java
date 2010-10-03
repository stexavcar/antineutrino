package org.neutrino.runtime;

import java.util.List;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;
import org.neutrino.syntax.Annotation;

@Growable(RProtocol.TAG)
public class RProtocol extends RValue implements ISeedable {

  private static final TypeId TYPE_ID = TypeId.get("Protocol");
  static final String TAG = "org::neutrino::runtime::RProtocol";

  public @SeedMember List<Annotation> annotations;
  public @SeedMember String name;
  public @SeedMember TypeId instanceTypeId;

  public RProtocol(List<Annotation> annotations, String name) {
    this.annotations = annotations;
    this.name = name;
    this.instanceTypeId = createTypeId();
  }

  public RProtocol() { }

  private TypeId createTypeId() {
    String key = name;
    for (Annotation annot : annotations) {
      if (annot.getTag().equals(Native.ANNOTATION)) {
        key = ((RString) annot.getArgument(0)).getValue();
        break;
      }
    }
    return TypeId.get(key);
  }

  public void initialize() { }

  public String getName() {
    return name;
  }

  public TypeId getInstanceTypeId() {
    return this.instanceTypeId;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return "#<a Protocol: " + name + ">";
  }

}
