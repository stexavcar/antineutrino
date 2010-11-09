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

  private static abstract class FlagInfo {

    private final Field field;

    public FlagInfo(Field field) {
      this.field = field;
    }

    protected Field getField() {
      return field;
    }

    public abstract int process(int index, String[] args);

  }

  private static class StringFlagInfo extends FlagInfo {

    public StringFlagInfo(Field field) {
      super(field);
    }

    @Override
    public int process(int index, String[] args) {
      setValue(args[index]);
      return index + 1;
    }

    private void setValue(String value) {
      try {
        getField().set(null, value);
      } catch (IllegalAccessException iae) {
        throw new RuntimeException(iae);
      }
    }

  }

  private static class BooleanFlagInfo extends FlagInfo {

    public BooleanFlagInfo(Field field) {
      super(field);
    }

    @Override
    public int process(int index, String[] args) {
      toggleValue();
      return index;
    }

    private void toggleValue() {
      try {
        boolean value = getField().getBoolean(null);
        getField().set(0, !value);
      } catch (IllegalAccessException iae) {
        throw new RuntimeException(iae);
      }
    }

  }

  private static class ListFlagInfo extends FlagInfo {

    private final List<String> list;

    @SuppressWarnings("unchecked")
    public ListFlagInfo(Field field) {
      super(field);
      try {
        list = (List<String>) field.get(null);
      } catch (IllegalAccessException iae) {
        throw new RuntimeException(iae);
      }
    }

    @Override
    public int process(int index, String[] args) {
      list.add(args[index]);
      return index + 1;
    }

  }

  public static List<String> parseArguments(String[] args, Class<?>... klasses) {
    Map<String, FlagInfo> flags = new HashMap<String, FlagInfo>();
    for (Class<?> klass : klasses) {
      for (Field field : klass.getDeclaredFields()) {
        Flag flag = field.getAnnotation(Flag.class);
        if (flag != null) {
          String setName = flag.value();
          if (setName.length() == 0)
            setName = field.getName();
          if (field.getType() == String.class) {
            flags.put(setName, new StringFlagInfo(field));
          } else if (field.getType() == Boolean.TYPE) {
            flags.put(setName, new BooleanFlagInfo(field));
          } else if (field.getType() == List.class) {
            flags.put(setName, new ListFlagInfo(field));
          } else {
            assert false: setName;
          }
        }
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
        i = handler.process(i, args);
      } else {
        result.add(arg);
      }
    }
    return result;
  }

}
