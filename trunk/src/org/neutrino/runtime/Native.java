package org.neutrino.runtime;

import java.io.File;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

import org.neutrino.plankton.ISeedable;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

@Growable(Native.TAG)
public class Native implements ISeedable {

  public static class Arguments {

    private Frame frame;
    private int argc;

    public RValue getThis() {
      Stack<RValue> stack = frame.parent.stack;
      return stack.get(stack.size() - argc + 1);
    }

    public RValue getArgument(int index) {
      Stack<RValue> stack = frame.parent.stack;
      return stack.get(stack.size() - argc + index + 2);
    }

    public RValue getFunctionArgument(int index) {
      return getArgument(index - 1);
    }

    public void prepare(Frame frame, int argc) {
      this.frame = frame;
      this.argc = argc;
    }

    public Frame getFrame() {
      return frame;
    }

  }

  static final String TAG = "org::neutrino::runtime::Native";
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

  @Marker("int|int") static final Impl INT_BITWISE_OR = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return new RInteger(a.getValue() | b.getValue());
    }
  };

  @Marker("int<<int") static final Impl INT_SHL = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return new RInteger(a.getValue() << b.getValue());
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

  @Marker("int*int") static final Impl INT_MULT = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return new RInteger(a.getValue() * b.getValue());
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

  @Marker("str+str") static final Impl STRING_PLUS = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RString self = (RString) args.getThis();
      RString other = (RString) args.getArgument(0);
      return new RString(self.getValue() + other.getValue());
    }
  };

  @Marker("str=str") static final Impl STRING_EQ = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RString self = (RString) args.getThis();
      RString other = (RString) args.getArgument(0);
      return RBoolean.get(self.getValue().equals(other.getValue()));
    }
  };

  @Marker("ord2str") static final Impl ORD_2_STR = new Impl() {
    @Override
    public RValue call(Arguments args) {
      int value = ((RInteger) args.getFunctionArgument(0)).getValue();
      if (value == 0)
        return new RString("");
      return new RString(Character.toString((char) value));
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

  @Marker("ref") static final Impl NEW_REF = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RValue value = args.getFunctionArgument(0);
      return new RReference(value);
    }
  };

  @Marker("ref.get") static final Impl REF_GET = new Impl() {
    @Override
    public RValue call(Arguments args) {
      return ((RReference) args.getThis()).getValue();
    }
  };

  @Marker("ref.set") static final Impl REF_SET = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RValue value = args.getArgument(0);
      ((RReference) args.getThis()).setValue(value);
      return value;
    }
  };

  @Marker("mutprimarr") static final Impl MUT_PRIM_ARR = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger size = (RInteger) args.getArgument(0);
      return new RMutablePrimitiveArray(size.getValue());
    }
  };

  @Marker("mutarr.set") static final Impl MUTARR_SET = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RMutablePrimitiveArray self = (RMutablePrimitiveArray) args.getThis();
      RInteger index = (RInteger) args.getArgument(0);
      RValue value = args.getArgument(1);
      self.set(index.getValue(), value);
      return value;
    }
  };

  @Marker("array[]") static final Impl ARRAY_GET = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RPrimitiveArray arr = (RPrimitiveArray) args.getThis();
      RInteger index = (RInteger) args.getArgument(0);
      return arr.get(index.getValue());
    }
  };

  @Marker("array.length") static final Impl ARRAY_LENGTH = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RPrimitiveArray arr = (RPrimitiveArray) args.getThis();
      return new RInteger(arr.getLength());
    }
  };

  @Marker("byte_array[]") static final Impl BYTE_ARRAY_GET = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RByteArray arr = (RByteArray) args.getThis();
      RInteger index = (RInteger) args.getArgument(0);
      return new RInteger((arr.get(index.getValue()) + 0x100) & 0xFF);
    }
  };

  @Marker("byte_array.length") static final Impl BYTE_ARRAY_LENGTH = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RByteArray arr = (RByteArray) args.getThis();
      return new RInteger(arr.getLength());
    }
  };

  @Marker("file.read") static final Impl FILE_READ = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RFile file = (RFile) args.getThis();
      byte[] data = file.read();
      return new RByteArray(data);
    }
  };

  @Marker("print") static final Impl PRINT = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RString value = (RString) args.getFunctionArgument(0);
      System.out.println(value.getValue());
      return RNull.getInstance();
    }
  };

  @Marker("select") static final Impl SELECT = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RBoolean cond = (RBoolean) args.getFunctionArgument(0);
      if (cond.getValue()) {
        return args.getFunctionArgument(1);
      } else {
        return args.getFunctionArgument(2);
      }
    }
  };

  @Marker("fail") static final Impl FAIL = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RValue message = args.getFunctionArgument(0);
      throw new RuntimeException("Fail: " + message.toExternalString());
    }
  };

  @Marker("open_file") static final Impl OPEN_FILE = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RString name = (RString) args.getFunctionArgument(0);
      File file = new File(name.getValue());
      return new RFile(file);
    }
  };

  @Marker("exit") static final Impl EXIT = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger value = (RInteger) args.getFunctionArgument(0);
      System.exit(value.getValue());
      return RNull.getInstance();
    }
  };

  @Marker("cont.fire") static final Impl CONT_FIRE = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RContinuation target = (RContinuation) args.getThis();
      RValue value = args.getArgument(0);
      while (args.frame.marker != target)
        args.frame = args.frame.parent;
      args.frame.stack.pop();
      args.frame.stack.pop();
      return value;
    }
  };

  @Marker("are_identical") static final Impl ARE_IDENTICAL = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RValue self = args.getThis();
      RValue that = args.getArgument(0);
      return RBoolean.get(self == that);
    }
  };

  @Marker("obj.to_string") static final Impl OBJ_TO_STRING = new Impl() {
    @Override
    public RValue call(Arguments args) {
      return new RString(args.getThis().toExternalString());
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
