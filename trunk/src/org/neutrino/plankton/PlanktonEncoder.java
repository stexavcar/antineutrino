package org.neutrino.plankton;

import java.io.IOException;
import java.io.OutputStream;
import java.util.IdentityHashMap;
import java.util.List;
import java.util.Map;

public class PlanktonEncoder {

  public static final int kIntTag = 0;
  public static final int kStringTag = 1;
  public static final int kNullTag = 2;
  public static final int kTrueTag = 3;
  public static final int kFalseTag = 4;
  public static final int kMapTag = 5;
  public static final int kArrayTag = 6;

  private final Map<Object, Integer> seen = new IdentityHashMap<Object, Integer>();
  private final LowLevelEncoder out;

  public PlanktonEncoder(OutputStream out) {
    this.out = new LowLevelEncoder(out);
  }

  public void write(Object obj) throws IOException {
    internalWrite(obj);
    out.flush();
  }

  public void internalWrite(Object obj) throws IOException {
    if (obj == null) {
      writeNull();
    } else if (obj instanceof Integer) {
      this.writeInteger((Integer) obj);
    } else if (obj instanceof String) {
      this.writeString((String) obj);
    } else if (obj instanceof Map<?, ?>) {
      this.writeMap((Map<?, ?>) obj);
    } else if (obj instanceof List<?>) {
      this.writeList((List<?>) obj);
    }
  }

  private void writeList(List<?> obj) throws IOException {
    out.write(kArrayTag);
    out.writeUnsigned(obj.size());
    for (Object elm : obj)
      internalWrite(elm);
  }

  private void writeMap(Map<?, ?> obj) throws IOException {
    out.write(kMapTag);
    out.writeUnsigned(obj.size());
    for (Map.Entry<?, ?> entry : obj.entrySet()) {
      internalWrite(entry.getKey());
      internalWrite(entry.getValue());
    }
  }

  private void writeNull() throws IOException {
    out.write(kNullTag);
  }

  private void writeInteger(int value) throws IOException {
    out.write(kIntTag);
    out.writeSigned(value);
  }

  private void writeString(String obj) throws IOException {
    out.write(kStringTag);
    out.writeUnsigned(obj.length());
    for (byte b : obj.getBytes())
      out.write(((int) b) & 0xFF);
  }

}
