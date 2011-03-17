package org.neutrino.plankton;

import org.neutrino.runtime.RValue;

public interface IBuiltinObjectIndex {

  public RValue getValue(Object key);

  public Object getKey(RValue value);

}
