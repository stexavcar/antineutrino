package org.neutrino.runtime;

import java.util.HashMap;
import java.util.Map;

public class Environment {

  private final Map<String, MultiMethod> methods = new HashMap<String, MultiMethod>() {{
    put("+", new MultiMethod());
  }};

}
