package org.neutrino.runtime;

import java.io.File;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

import org.javatrino.bytecode.Opcode;
import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Universe;
import org.neutrino.plankton.Store;

public class Native {

  public static class Arguments {

    private Frame frame;
    private int argc;
    private Universe universe;

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

    public void prepare(Frame frame, int argc, Universe universe) {
      this.frame = frame;
      this.argc = argc;
      this.universe = universe;
    }

    public Frame getFrame() {
      return frame;
    }

    public Universe getUniverse(){
      return universe;
    }

  }

  static final String TAG = "org::neutrino::runtime::Native";
  public static final String ANNOTATION = "native";

  private static abstract class Impl {
    public abstract RValue call(Arguments args);
  }

  public @Store String name;
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
      return RInteger.get(a.getValue() + b.getValue());
    }
  };

  @Marker("int|int") static final Impl INT_BITWISE_OR = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RInteger.get(a.getValue() | b.getValue());
    }
  };

  @Marker("int&int") static final Impl INT_BITWISE_AND = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RInteger.get(a.getValue() & b.getValue());
    }
  };

  @Marker("int<<int") static final Impl INT_SHL = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RInteger.get(a.getValue() << b.getValue());
    }
  };

  @Marker("int>>int") static final Impl INT_SHR = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RInteger.get(a.getValue() >> b.getValue());
    }
  };

  @Marker("int-int") static final Impl INT_MINUS = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RInteger.get(a.getValue() - b.getValue());
    }
  };

  @Marker("int*int") static final Impl INT_MULT = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RInteger.get(a.getValue() * b.getValue());
    }
  };

  @Marker("int%int") static final Impl INT_MOD = new Impl() {
    @Override
    public RValue call(Arguments args) {
      int a = ((RInteger) args.getThis()).getValue();
      int b = ((RInteger) args.getArgument(0)).getValue();
      if (b == 0) {
        throw new InterpreterError(new ArithmeticException(), args.frame,
            args.getUniverse());
      }
      return RInteger.get(a % b);
    }
  };

  @Marker("int/int") static final Impl INT_DIV = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger a = (RInteger) args.getThis();
      RInteger b = (RInteger) args.getArgument(0);
      return RInteger.get(a.getValue() / b.getValue());
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

  @Marker("str.len") static final Impl STRING_LENGTH = new Impl() {
    @Override
    public RValue call(Arguments args) {
      return RInteger.get(((RString) args.getThis()).getValue().length());
    }
  };

  @Marker("str[ascii]") static final Impl STRING_GET_ASCII = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RString self = (RString) args.getThis();
      RInteger index = (RInteger) args.getArgument(0);
      return RInteger.get(self.getValue().charAt(index.getValue()));
    }
  };

  @Marker("str.implode") static final Impl STRING_IMPLODE = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RArray elms = (RArray) args.getArgument(0);
      int length = ((RInteger) args.getArgument(1)).getValue();
      String sep = ((RString) args.getArgument(2)).getValue();
      StringBuilder buf = new StringBuilder();
      for (int i = 0; i < length; i++) {
        if (i > 0) buf.append(sep);
        String part = ((RString) elms.get(i)).getValue();
        buf.append(part);
      }
      return new RString(buf.toString());
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

  @Marker("ref") static final Impl NEW_REF = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RValue value = args.getArgument(0);
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

  @Marker("mutprimbytarr") static final Impl MUT_PRIM_BYT_ARR = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RInteger size = (RInteger) args.getArgument(0);
      return new RMutableByteArray(size.getValue());
    }
  };

  @Marker("mutbytarr.set") static final Impl MUTBYTARR_SET = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RMutableByteArray self = (RMutableByteArray) args.getThis();
      RInteger index = (RInteger) args.getArgument(0);
      RInteger value = (RInteger) args.getArgument(1);
      self.set(index.getValue(), value.getValue());
      return value;
    }
  };

  @Marker("mutarr.set") static final Impl MUTARR_SET = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RMutablePrimitiveArray self = (RMutablePrimitiveArray) args.getThis();
      RInteger index = (RInteger) args.getArgument(0);
      if (index.getValue() >= self.getLength()) {
        throw new InterpreterError(new IndexOutOfBoundsException(), args.frame,
            args.getUniverse());
      }
      RValue value = args.getArgument(1);
      self.set(index.getValue(), value);
      return value;
    }
  };

  @Marker("array[]") static final Impl ARRAY_GET = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RPrimitiveArray arr = (RPrimitiveArray) args.getThis();
      int index = ((RInteger) args.getArgument(0)).getValue();
      if (index < 0 || index >= arr.getLength()) {
        throw new InterpreterError(new IndexOutOfBoundsException(), args.frame,
            args.getUniverse());
      }
      return arr.get(index);
    }
  };

  @Marker("array.length") static final Impl ARRAY_LENGTH = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RPrimitiveArray arr = (RPrimitiveArray) args.getThis();
      return RInteger.get(arr.getLength());
    }
  };

  @Marker("byte_array[1]") static final Impl BYTE_ARRAY_GET_1 = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RByteArray arr = (RByteArray) args.getThis();
      int index = ((RInteger) args.getArgument(0)).getValue();
      if (index < 0 || index >= arr.getLength()) {
        throw new InterpreterError(new IndexOutOfBoundsException(), args.frame,
            args.getUniverse());
      }
      return RInteger.get((arr.getByte(index) + 0x100) & 0xFF);
    }
  };

  @Marker("byte_array[4]") static final Impl BYTE_ARRAY_GET_4 = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RByteArray arr = (RByteArray) args.getThis();
      int index = ((RInteger) args.getArgument(0)).getValue();
      if (index < 0 || index + 3 >= arr.getLength()) {
        throw new InterpreterError(new IndexOutOfBoundsException(), args.frame,
            args.getUniverse());
      }
      byte[] bytes = arr.getBytes();
      int b0 = (bytes[index + 0] + 0x100) & 0xFF;
      int b1 = (bytes[index + 1] + 0x100) & 0xFF;
      int b2 = (bytes[index + 2] + 0x100) & 0xFF;
      int b3 = (bytes[index + 3] + 0x100) & 0xFF;
      return RInteger.get(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    }
  };

  @Marker("byte_array.length") static final Impl BYTE_ARRAY_LENGTH = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RByteArray arr = (RByteArray) args.getThis();
      return RInteger.get(arr.getLength());
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

  @Marker("file.write") static final Impl FILE_WRITE = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RFile file = (RFile) args.getThis();
      RByteArray data = (RByteArray) args.getArgument(0);
      file.write(data.getBytes());
      return RNull.getInstance();
    }
  };

  @Marker("file.writestring") static final Impl FILE_WRITE_STR = new Impl() {
	    @Override
	    public RValue call(Arguments args) {
	      RFile file = (RFile) args.getThis();
	      RString data = (RString) args.getArgument(0);
	      file.write(data.getValue().getBytes());
	      return RNull.getInstance();
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

  @Marker("fail") static final Impl FAIL = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RValue message = args.getFunctionArgument(0);
      throw new InterpreterError.Failure("Fail: " + message.toExternalString(),
          args.frame, args.getUniverse());
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

  @Marker("obj.hash") static final Impl OBJ_HASH = new Impl() {
    @Override
    public RValue call(Arguments args) {
      return RInteger.get(args.getArgument(0).hashCode());
    }
  };

  private static final byte[] TRAMPOLINE_CODE = new byte[] {
    Opcode.kCall, 0, 0, 0, Opcode.kReturnFromApply
  };

  @Marker("apply") static final Impl APPLY = new Impl() {
    @Override
    public RValue call(Arguments args) {
      RString name = (RString) args.getFunctionArgument(0);
      RArray values = (RArray) args.getFunctionArgument(1);
      Universe universe = args.getUniverse();
      Stack<RValue> stack = new Stack<RValue>();
      int argc = values.getLength();
      for (int i = 0; i < argc; i++)
        stack.push(values.get(i));
      Lambda method = universe.lookupMethod(name.getValue(), argc, stack);
      if (method == null)
        throw new InterpreterError.MethodNotFound(args.frame, args.getUniverse());
      Frame trampoline = new Frame(args.frame, values.get(0),
          new CodeBundle(TRAMPOLINE_CODE, Arrays.<Object>asList(name.getValue()),
              0));
      trampoline.stack = stack;
      args.frame = new Frame(trampoline, values.get(0), method.getCode());
      return null;
    }
  };

  @SuppressWarnings("serial")
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
