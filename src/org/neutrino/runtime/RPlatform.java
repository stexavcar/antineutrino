package org.neutrino.runtime;

import org.neutrino.pib.Module;
import org.neutrino.pib.Universe;


public class RPlatform extends RAbstractValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("Platform");

  private final Universe universe;

  public RPlatform(Universe universe) {
    this.universe = universe;
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

  @Override
  public String toString() {
    return "#<platform>";
  }

  public RModule lookupModule(RValue name) {
    Module module = universe.getModule(name);
    return (module == null) ? null : module.getWrapper();
  }

}
