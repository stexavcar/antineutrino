package org.neutrino.runtime;

public abstract class RArray extends RValue {

  public abstract int getLength();
  public abstract RValue get(int index);

}
