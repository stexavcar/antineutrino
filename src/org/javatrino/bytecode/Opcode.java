package org.javatrino.bytecode;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;

/**
 * Opcodes used to encode syntax trees in platform independent binaries.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Opcode {

  public static class OpcodeInfo {

    private final String name;
    private final ArgType[] args;

    public OpcodeInfo(Info info, String name) {
      this.name = name;
      this.args = info.value();
    }

    public int getSize() {
      return args.length + 1;
    }

    public ArgType[] getArguments() {
      return args;
    }

    public String getName() {
      return name;
    }

  }

  public enum ArgType {
    LIT, NUM
  }

  @Retention(RetentionPolicy.RUNTIME)
  private @interface Info {
    public ArgType[] value();
  }

  public static final int kCallSize = 4;

  @Info({ArgType.LIT, ArgType.NUM, ArgType.NUM}) public static final int kCall = 3;
  @Info({}) public static final int kReturn = 4;
  @Info({}) public static final int kNull = 5;
  @Info({}) public static final int kTrue = 6;
  @Info({ArgType.LIT}) public static final int kLiteral = 7;
  @Info({ArgType.LIT, ArgType.NUM, ArgType.NUM}) public static final int kCallNative = 8;
  @Info({}) public static final int kPop = 9;
  @Info({ArgType.LIT}) public static final int kGlobal = 10;
  @Info({}) public static final int kFalse = 11;
  @Info({ArgType.NUM}) public static final int kArgument = 12;
  @Info({ArgType.NUM}) public static final int kOuter = 13;
  @Info({ArgType.NUM}) public static final int kLocal = 14;
  @Info({ArgType.NUM}) public static final int kStoreLocal = 15;
  @Info({}) public static final int kTerminate = 16;
  @Info({ArgType.LIT, ArgType.NUM}) public static final int kNew = 17;
  @Info({ArgType.NUM}) public static final int kGetter = 18;
  @Info({ArgType.NUM, ArgType.NUM}) public static final int kWith1cc = 19;
  @Info({ArgType.NUM}) public static final int kNewArray = 20;
  @Info({}) public static final int kReturnFromApply = 21;
  @Info({ArgType.NUM}) public static final int kBlock = 22;
  @Info({ArgType.NUM, ArgType.NUM, ArgType.NUM}) public static final int kDefineLocal = 23;
  @Info({ArgType.NUM}) public static final int kSetter = 24;
  @Info({ArgType.LIT}) public static final int kAddIntrinsics = 25;
  @Info({}) public static final int kNewObject = 26;
  @Info({ArgType.LIT}) public static final int kSetField = 27;
  @Info({ArgType.NUM}) public static final int kTagWithProtocols = 28;
  @Info({ArgType.LIT}) public static final int kGetField = 29;
  @Info({}) public static final int kDup = 30;
  @Info({ArgType.NUM, ArgType.NUM}) public static final int kWithEscape = 31;

  private static final Map<Integer, OpcodeInfo> infos = new HashMap<Integer, OpcodeInfo>() {{
    try {
      for (Field field : Opcode.class.getFields()) {
        Info info = field.getAnnotation(Info.class);
        if (info != null) {
          String name = field.getName().substring(1);
          put(field.getInt(null), new OpcodeInfo(info, name));
        }
      }
    } catch (IllegalAccessException iae) {
      throw new RuntimeException(iae);
    }
  }};

  public static OpcodeInfo getInfo(int code) {
    return infos.get(code);
  }

}
