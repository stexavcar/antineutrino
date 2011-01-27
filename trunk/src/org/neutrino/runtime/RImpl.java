package org.neutrino.runtime;

import java.util.ArrayList;
import java.util.Collections;
import java.util.IdentityHashMap;
import java.util.List;

import org.javatrino.ast.Method;

public class RImpl {

  public final List<RProtocol> protos;
  public final List<Method> intrinsics;
  public List<TypeId> typeIds = Collections.<TypeId>emptyList();

  private final IdentityHashMap<RProtocol, RImpl> addedProtos
      = new IdentityHashMap<RProtocol, RImpl>();

  private final IdentityHashMap<List<Method>, RImpl> addedIntrinsics
      = new IdentityHashMap<List<Method>, RImpl>();

  public RImpl() {
    protos = new ArrayList<RProtocol>();
    intrinsics = new ArrayList<Method>();
  }

  public RImpl(List<RProtocol> protos, List<Method> intrinsics) {
    this.protos = protos;
    this.intrinsics = intrinsics;
  }

  public RImpl addProtocol(RProtocol proto) {
    RImpl next = addedProtos.get(proto);
    if (next == null) {
      List<RProtocol> newProtos = new ArrayList<RProtocol>(protos);
      newProtos.add(proto);
      next = new RImpl(newProtos, intrinsics);
      addedProtos.put(proto, next);
    }
    return next;
  }

  public RImpl addIntrinsics(List<Method> values) {
    RImpl next = addedIntrinsics.get(values);
    if (next == null) {
      List<Method> newIntrinsics = new ArrayList<Method>(values);
      newIntrinsics.addAll(values);
      next = new RImpl(protos, newIntrinsics);
      addedIntrinsics.put(values, next);
    }
    return next;
  }

  public List<RProtocol> getProtocols() {
    return protos;
  }

  public List<Method> getIntrinsics() {
    return intrinsics;
  }

  public Iterable<TypeId> getTypeIds() {
    if (this.typeIds == null) {
      typeIds = new ArrayList<TypeId>();
      for (RProtocol proto : protos)
        typeIds.add(proto.getInstanceTypeId());
    }
    return this.typeIds;
  }

}
