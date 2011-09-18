package org.neutrino.runtime;

import java.util.ArrayList;
import java.util.List;

import org.neutrino.pib.Module;
import org.neutrino.plankton.Store;

public class RModule extends RAbstractValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("Module");

  public @Store Module owner;
  public @Store RValue name;
  public @Store List<RModule> delegates;

  public RModule(RValue name) {
    this.name = name;
    this.delegates = new ArrayList<RModule>();
  }

  public void setOwner(Module owner) {
    assert this.owner == null;
    this.owner = owner;
  }

  public RModule() { }

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
    return "#<module " + name.toExternalString() + ">";
  }

  public RValue getName() {
    return name;
  }

  public void addDelegateNamespace(RModule value) {
    delegates.add(value);
  }

}
