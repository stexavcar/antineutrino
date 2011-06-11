package org.neutrino.runtime;

public interface RPrimitiveArray extends RPrimitiveMap {

  public abstract int getLength();
  public abstract RValue get(int index);

}
