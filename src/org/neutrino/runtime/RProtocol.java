package org.neutrino.runtime;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.pib.Universe;
import org.neutrino.plankton.Store;
import org.neutrino.syntax.Annotation;

public class RProtocol extends RAbstractValue {

  public @Store List<Annotation> annotations;
  public @Store RValue id;
  public @Store RValue displayName;

  public RProtocol(List<Annotation> annotations, RValue id,
      RValue displayName) {
    this.annotations = annotations;
    this.id = id;
    this.displayName = displayName;
  }

  public RProtocol() { }

  public RValue getId() {
    return id;
  }

  public RValue getName() {
    return displayName;
  }

  public List<Annotation> getAnnotations() {
    return this.annotations;
  }

  private static RProtocol[] protoCache = null;
  @Override
  public RProtocol[] getTypeIds() {
    if (protoCache == null)
      protoCache = getCanonicals("Protocol");
    return protoCache;
  }

  @Override
  public String toString() {
    return "%" + id.toExternalString();
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

  private static final Map<String, RProtocol> CANONICAL = new HashMap<String, RProtocol>();

  public static RProtocol getCanonical(String name) {
    RProtocol result = CANONICAL.get(name);
    if (result == null) {
      result = (RProtocol) Universe.getBuiltinIndex().getValue(name);
      assert result != null : name;
      CANONICAL.put(name, result);
    }
    return result;
  }

  public static RProtocol[] getCanonicals(String... names) {
    RProtocol[] result = new RProtocol[names.length];
    for (int i = 0; i < names.length; i++)
      result[i] = getCanonical(names[i]);
    return result;
  }

}
