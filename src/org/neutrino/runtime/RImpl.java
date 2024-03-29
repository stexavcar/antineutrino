package org.neutrino.runtime;

import java.util.ArrayList;
import java.util.IdentityHashMap;
import java.util.List;

import org.javatrino.ast.Method;
import org.neutrino.plankton.Store;

public class RImpl {

  public @Store List<RProtocol> protos;
  public @Store List<Method> intrinsics;
  public RProtocol[] protoArray = null;

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

  public RProtocol[] getProtocolArray() {
    if (this.protoArray == null) {
      List<RProtocol> ids = new ArrayList<RProtocol>();
      for (RProtocol proto : protos)
        ids.add(proto);
      protoArray = ids.toArray(new RProtocol[ids.size()]);
    }
    return this.protoArray;
  }

}
