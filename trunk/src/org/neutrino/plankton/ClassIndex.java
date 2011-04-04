package org.neutrino.plankton;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.PlanktonEncoder.EnumEncoder;
import org.neutrino.plankton.PlanktonEncoder.IObjectEncoder;
import org.neutrino.plankton.PlanktonEncoder.PojoEncoder;

public class ClassIndex {

  private final Map<String, IObjectEncoder> map = new HashMap<String, IObjectEncoder>();
  private final List<IObjectEncoder> encoders = new ArrayList<IObjectEncoder>();

  public void add(Class<?> klass) {
    @SuppressWarnings("unchecked")
    IObjectEncoder encoder = klass.isEnum()
        ? new EnumEncoder((Class<? extends Enum<?>>) klass)
        : new PojoEncoder(klass);
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
