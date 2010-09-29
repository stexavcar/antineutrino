package org.neutrino.runtime;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

@Growable(Native.TAG)
public class Native implements ISeedable {

  public static class Arguments {

    private Frame frame;
    private int argc;

    public RValue getThis() {
      Stack<RValue> stack = frame.stack;
      return stack.get(stack.size() - argc);
    }

    public RValue getArgument(int index) {
      Stack<RValue> stack = frame.stack;
      return stack.get(stack.size() - argc + index + 1);
    }

    public void prepare(Frame frame, int argc) {
      this.frame = frame;
      this.argc = argc;
    }

  }

  private static final String TAG = "org::neutrino::runtime::Native";
  public static final String ANNOTATION = "native";

  private static abstract class Impl {
    public abstract RValue call(Arguments args);
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

  public RValue call(Arguments args) {
    if (impl == null) {
      impl = IMPLS.get(name);
      assert impl != null : "Undefined native " + name;
    }
    return impl.call(args);
  }

  @Override
  public String toString() {
    return "#<a Native: " + name + ">";
  }

  @Retention(RetentionPolicy.RUNTIME)
  private static @interface Marker {
    public String value();
  }

  @Marker("int+int") static final Impl INT_PLUS = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return new RInteger(a.getValue() + b.getValue());
    }
  };

  @Marker("int-int") static final Impl INT_MINUS = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return new RInteger(a.getValue() - b.getValue());
    }
  };

  @Marker("int<int") static final Impl INT_LT = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RBoolean.get(a.getValue() < b.getValue());
    }
  };

  @Marker("int=int") static final Impl INT_EQ = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RBoolean.get(a.getValue() == b.getValue());
    }
  };

  @Marker("null=any") static final Impl NULL_EQ = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RNull self = (RNull) args.getThis();
      return RBoolean.get(args.getArgument(0) == self);
    }
  };

  @Marker("!bool") static final Impl BOOL_NOT = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RBoolean self = (RBoolean) args.getThis();
      return RBoolean.get(!self.getValue());
    }
  };

  @Marker("print") static final Impl PRINT = new Impl() {
    @Override
    public RValue call(Arguments args) {
      System.out.println(args.getArgument(0));
      return RNull.getInstance();
    }
  };

  @Marker("assert_true") static final Impl ASSERT_TRUE = new Impl() {
    @Override
    public RValue call(Arguments args) {
      if (args.getArgument(0) != RBoolean.getTrue()) {
        throw new AssertionError("Assertion failed.");
      }
      return RNull.getInstance();
    }
  };

  @Marker("select") static final Impl IF = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RBoolean cond = (RBoolean) args.getArgument(0);
      if (cond.getValue()) {
        return args.getArgument(1);
      } else {
        return args.getArgument(2);
      }
    }
  };

  private static final Map<String, Impl> IMPLS = new HashMap<String, Impl>() {{
    try {
      for (Field field : Native.class.getDeclaredFields()) {
        Marker marker = field.getAnnotation(Marker.class);
        if (marker != null) {
          String value = marker.value();
          put(value, (Impl) field.get(null));
        }
      }
    } catch (IllegalAccessException iae) {
      throw new RuntimeException(iae);
    }
  }};

}