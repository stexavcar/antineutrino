package org.neutrino.runtime;

import java.util.HashMap;
import java.util.Map;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(Native.TAG)
public class Native implements ISeedable {

  private static final String TAG = "org::neutrino::runtime::Native";
  public static final String ANNOTATION = "native";

  private static abstract class Impl {
    public abstract RValue call(Frame frame);
  }

  public @SeedMember String name;
  private Impl impl = null;

  public Native(String name) {
    this.name = name;
  }

  public Native() { }

  public static Native get(String name) {
    return new Native(name);
  }

  public RValue call(Frame frame) {
    if (impl == null)
      impl = IMPLS.get(name);
    return impl.call(frame);
  }

  private static final Map<String, Impl> IMPLS = new HashMap<String, Impl>() {{
    put("int+any", new Impl() {
      @Override
      public RValue call(Frame frame) {
        RInteger a = (RInteger) frame.getArgument(1);
        RInteger b = (RInteger) frame.getArgument(0);
        return new RInteger(a.getValue() + b.getValue());
      }
    });
    put("int-any", new Impl() {
      @Override
      public RValue call(Frame frame) {
        RInteger a = (RInteger) frame.getArgument(1);
        RInteger b = (RInteger) frame.getArgument(0);
        return new RInteger(a.getValue() - b.getValue());
      }
    });
  }};

}
