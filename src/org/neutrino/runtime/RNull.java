package org.neutrino.runtime;

import org.neutrino.plankton.Atomic;
import org.neutrino.plankton.Generator;

@Atomic
public class RNull extends RValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("null");
  private static final RNull INSTANCE = new RNull();

  public RNull() { }

  public static RNull getInstance() {
    return INSTANCE;
  }

  @Generator
  public static RValue get() {
    return INSTANCE;
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  @Override
  public String toString() {
    return "#n";
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
