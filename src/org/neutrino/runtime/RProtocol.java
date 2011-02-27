package org.neutrino.runtime;

import java.util.List;

import org.neutrino.plankton.Store;
import org.neutrino.syntax.Annotation;

public class RProtocol extends RValue {

  public @Store List<Annotation> annotations;
  public @Store String id;
  public @Store String displayName;
  public @Store TypeId protocolTypeId;
  public @Store TypeId instanceTypeId;

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

  public String getId() {
    return id;
  }

  public String getName() {
    return displayName;
  }

  public TypeId getInstanceTypeId() {
    return this.instanceTypeId;
  }

  public TypeId getProtocolTypeId() {
    return this.protocolTypeId;
  }

  private TypeId[] typeIdCache = null;
  @Override
  public TypeId[] getTypeIds() {
    if (typeIdCache == null)
      typeIdCache = new TypeId[] { protocolTypeId };
    return typeIdCache;
  }

  @Override
  public String toString() {
    return "#<a Protocol: " + id + ">";
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
