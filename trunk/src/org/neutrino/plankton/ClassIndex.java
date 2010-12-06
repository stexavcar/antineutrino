package org.neutrino.plankton;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.PlanktonEncoder.ObjectEncoder;

public class ClassIndex {

  private final Map<String, ObjectEncoder> map = new HashMap<String, ObjectEncoder>();
  private final List<ObjectEncoder> encoders = new ArrayList<ObjectEncoder>();

  public void add(Class<?> klass) {
    ObjectEncoder encoder = new ObjectEncoder(klass);
    encoders.add(encoder);
    map.put(klass.getName(), encoder);
  }

  List<ObjectEncoder> getEncoders() {
    return encoders;
  }

  ObjectEncoder getEncoder(String name) {
    return map.get(name);
  }

}
