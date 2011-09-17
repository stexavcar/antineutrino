package org.javatrino.utils;

public class Utils {

  public static String join(Iterable<?> elms, String sep) {
    StringBuilder buf = new StringBuilder();
    boolean first = true;
    for (Object elm : elms) {
      if (first) {
        first = false;
      } else {
        buf.append(", ");
      }
      buf.append(elm);
    }
    return buf.toString();
  }

}
