package org.neutrino.pib;

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
    LITERAL, NUMBER
  }

  @Retention(RetentionPolicy.RUNTIME)
  private @interface Info {
    public ArgType[] value();
  }

  @Info({ArgType.LITERAL, ArgType.NUMBER}) public static final int kCall = 3;
  @Info({}) public static final int kReturn = 4;
  @Info({}) public static final int kNull = 5;
  @Info({}) public static final int kTrue = 6;
  @Info({ArgType.LITERAL}) public static final int kPush = 7;
  @Info({ArgType.LITERAL, ArgType.NUMBER}) public static final int kCallNative = 8;
  @Info({}) public static final int kPop = 9;
  @Info({ArgType.LITERAL}) public static final int kGlobal = 10;
  @Info({}) public static final int kFalse = 11;
  @Info({ArgType.NUMBER}) public static final int kArgument = 12;
  @Info({ArgType.NUMBER}) public static final int kOuter = 13;
  @Info({ArgType.NUMBER}) public static final int kLocal = 14;
  @Info({ArgType.NUMBER}) public static final int kStoreLocal = 15;
  @Info({}) public static final int kTerminate = 16;
  @Info({ArgType.LITERAL, ArgType.NUMBER}) public static final int kNew = 17;
  @Info({ArgType.NUMBER}) public static final int kField = 18;

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
