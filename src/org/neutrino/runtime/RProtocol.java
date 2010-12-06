package org.neutrino.runtime;

import java.util.List;

import org.neutrino.oldton.ISeedable;
import org.neutrino.oldton.annotations.Growable;
import org.neutrino.oldton.annotations.SeedMember;
import org.neutrino.plankton.Store;
import org.neutrino.syntax.Annotation;

@Growable(RProtocol.TAG)
public class RProtocol extends RValue implements ISeedable {

  static final String TAG = "org::neutrino::runtime::RProtocol";

  public @Store @SeedMember List<Annotation> annotations;
  public @Store @SeedMember String id;
  public @Store @SeedMember String displayName;
  public @Store @SeedMember TypeId protocolTypeId;
  public @Store @SeedMember TypeId instanceTypeId;

  public RProtocol(List<Annotation> annotations, String id,
      String displayName) {
    this.annotations = annotations;
    this.id = id;
    this.displayName = displayName;
    String typeIdKey = getTypeIdKey();
    this.protocolTypeId = TypeId.get(typeIdKey + ".protocol");
    this.instanceTypeId = TypeId.get(typeIdKey);
  }

  public RProtocol() { }

  private String getTypeIdKey() {
    for (Annotation annot : annotations) {
      if (annot.getTag().equals(Native.ANNOTATION))
        return ((RString) annot.getArgument(0)).getValue();
    }
    return id;
  }

  public void initialize() { }

  public String getId() {
    return id;
  }

  public String getName() {
    return displayName;
  }

  public TypeId getInstanceTypeId() {
    return this.instanceTypeId;
  }

  @Override
  public TypeId getTypeId() {
    return this.protocolTypeId;
  }

  @Override
  public String toString() {
    return "#<a Protocol: " + id + ">";
  }

}
