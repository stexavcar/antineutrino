package org.neutrino.plankton;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.PlanktonEncoder.IObjectEncoder;
import org.neutrino.plankton.PlanktonEncoder.ObjectEncoder;

public class ClassIndex {

  private final Map<String, IObjectEncoder> map = new HashMap<String, IObjectEncoder>();
  private final List<IObjectEncoder> encoders = new ArrayList<IObjectEncoder>();

  public void add(Class<?> klass) {
    ObjectEncoder encoder = new ObjectEncoder(klass);
    encoders.add(encoder);
    map.put(klass.getName(), encoder);
  }

  public void addEncoder(String name, IObjectEncoder encoder) {
    encoders.add(encoder);
    map.put(name, encoder);
  }

  public void addAll(ClassIndex index) {
    map.putAll(index.map);
    encoders.addAll(index.encoders);
  }

  List<IObjectEncoder> getEncoders() {
    return encoders;
  }

  IObjectEncoder getEncoder(String name) {
    return map.get(name);
  }

}
