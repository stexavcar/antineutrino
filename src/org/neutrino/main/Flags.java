package org.neutrino.main;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class Flags {

  @Retention(RetentionPolicy.RUNTIME)
  public @interface Flag {
    public String value() default "";
  }

  private static class FlagInfo {

    private final Field field;

    public FlagInfo(Field field) {
      this.field = field;
    }

    public void setValue(String value) {
      try {
        field.set(null, value);
      } catch (IllegalAccessException iae) {
        throw new RuntimeException(iae);
      }
    }

  }

  public static List<String> parseArguments(String[] args, Class<?> klass) {
    Map<String, FlagInfo> flags = new HashMap<String, FlagInfo>();
    for (Field field : klass.getDeclaredFields()) {
      Flag flag = field.getAnnotation(Flag.class);
      if (flag != null) {
        String setName = flag.value();
        if (setName.isEmpty())
          setName = field.getName();
        flags.put(setName, new FlagInfo(field));
      }
    }
    List<String> result = new ArrayList<String>();
    for (int i = 0; i < args.length;) {
      String arg = args[i++];
      if (arg.startsWith("--")) {
        String flagName = arg.substring(2);
        FlagInfo handler = flags.get(flagName);
        if (handler == null)
          throw new RuntimeException("Unknown flag " + flagName);
        handler.setValue(args[i++]);
      } else {
        result.add(arg);
      }
    }
    return result;
  }

}
