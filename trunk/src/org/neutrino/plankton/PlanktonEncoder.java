package org.neutrino.plankton;

import java.io.IOException;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.IdentityHashMap;
import java.util.List;
import java.util.Map;

public class PlanktonEncoder {

  public static final int kIntTag = 0;
  public static final int kStringTag = 1;
  public static final int kNullTag = 2;
  public static final int kTrueTag = 3;
  public static final int kFalseTag = 4;

  public static final int kMapTag = 6;
  public static final int kRefMapTag = 7;
  public static final int kArrayTag = 8;
  public static final int kRefArrayTag = 9;
  public static final int kRefTag = 10;
  public static final int kScopeTag = 11;
  public static final int kTemplateTag = 12;
  public static final int kPlaceholderTag = 13;
  public static final int kObjectTag = 14;
  public static final int kBlobTag = 15;

  private static class EncoderRecord {
    private final int index;
    private final int offset;
    public EncoderRecord(int index, int offset) {
      this.index = index;
      this.offset = offset;
    }
  }

  public static class ObjectEncoder {

    private final Class<?> klass;
    private final Map<String, Field> fieldMap = new HashMap<String, Field>();
    private final List<Field> fields = new ArrayList<Field>();

    public ObjectEncoder(Class<?> klass) {
      this.klass = klass;
      for (Field field : klass.getDeclaredFields()) {
        if (field.getAnnotation(Store.class) != null) {
          fieldMap.put(field.getName(), field);
          fields.add(field);
        }
      }
    }

    public boolean canHandle(Object obj) {
      return klass.isInstance(obj);
    }

    public Object newInstance() {
      try {
        return klass.newInstance();
      } catch (IllegalAccessException iae) {
        throw new RuntimeException(iae);
      } catch (InstantiationException ie) {
        throw new RuntimeException(ie);
      }
    }

    public void set(Object obj, String key, Object value) {
      try {
        fieldMap.get(key).set(obj, value);
      } catch (IllegalArgumentException e) {
        throw new RuntimeException(e);
      } catch (IllegalAccessException e) {
        throw new RuntimeException(e);
      }
    }

  }

  private final Map<Object, EncoderRecord> seen = new IdentityHashMap<Object, EncoderRecord>();
  private final LowLevelEncoder out;
  private final ClassIndex klasses;
  private int nextIndex = 0;

  public PlanktonEncoder(ClassIndex klasses) {
    this.out = new LowLevelEncoder();
    this.klasses = klasses;
  }

  public void write(Object obj) throws IOException {
    if (!klasses.getEncoders().isEmpty()) {
      writeTemplates(klasses.getEncoders());
    }
    internalWrite(obj);
    out.flush();
  }


  private void writeTemplates(List<ObjectEncoder> encoders) throws IOException {
    out.write(kScopeTag);
    out.writeUnsigned(0);
    out.writeUnsigned(encoders.size());
    for (ObjectEncoder encoder : encoders) {
      out.write(kObjectTag);
      writeString(encoder.klass.getName());
      out.write(kMapTag);
      out.writeUnsigned(encoder.fields.size());
      for (Field field : encoder.fields) {
        writeString(field.getName());
        out.write(kPlaceholderTag);
      }
    }
  }

  private int getCurrentIndex() {
    return nextIndex - 1;
  }

  public void internalWrite(Object obj) throws IOException {
    if (obj == null) {
      this.writeNull();
    } else if (obj instanceof Integer) {
      this.writeInteger((Integer) obj);
    } else if (obj instanceof String) {
      this.writeString((String) obj);
    } else if (obj instanceof byte[]) {
      this.writeBlob((byte[]) obj);
    } else {
      this.writeReferrableObject(obj);
    }
  }

  private void writeReferrableObject(Object obj) throws IOException {
    EncoderRecord record = seen.get(obj);
    if (record != null) {
      this.writeReference(record);
    } else if (obj instanceof Map<?, ?>) {
      nextIndex++;
      this.writeMap((Map<?, ?>) obj);
    } else if (obj instanceof List<?>) {
      nextIndex++;
      this.writeList((List<?>) obj);
    } else {
      int index = 0;
      for (ObjectEncoder encoder : klasses.getEncoders()) {
        if (encoder.canHandle(obj)) {
          nextIndex++;
          this.writeObject(encoder, obj, index);
          return;
        }
        index++;
      }
      assert false : obj.getClass();
    }
  }

  public void writeObject(ObjectEncoder encoder, Object obj, int index) throws IOException {
    int offset = out.write(kTemplateTag);
    int objIndex = getCurrentIndex();
    seen.put(obj, new EncoderRecord(objIndex, offset));
    out.writeUnsigned(index);
    for (Field field : encoder.fields) {
      Object value;
      try {
        value = field.get(obj);
      } catch (IllegalArgumentException e) {
        throw new RuntimeException(e);
      } catch (IllegalAccessException e) {
        throw new RuntimeException(e);
      }
      internalWrite(value);
    }
  }

  private void writeReference(EncoderRecord record) throws IOException {
    out.write(kRefTag);
    out.writeUnsigned(record.index);
  }

  private void writeList(List<?> obj) throws IOException {
    int offset = out.write(kArrayTag);
    int index = getCurrentIndex();
    seen.put(obj, new EncoderRecord(index, offset));
    out.writeUnsigned(obj.size());
    for (Object elm : obj)
      internalWrite(elm);
  }

  private void writeMap(Map<?, ?> obj) throws IOException {
    int offset = out.write(kMapTag);
    int index = getCurrentIndex();
    seen.put(obj, new EncoderRecord(index, offset));
    out.writeUnsigned(obj.size());
    for (Map.Entry<?, ?> entry : obj.entrySet()) {
      internalWrite(entry.getKey());
      internalWrite(entry.getValue());
    }
  }

  public void flush() throws IOException {
    out.flush();
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

  private void writeBlob(byte[] obj) throws IOException {
    out.write(kBlobTag);
    out.writeUnsigned(obj.length);
    for (byte b : obj)
      out.write(((int) b) & 0xFF);
  }

  public byte[] getBytes() {
    return out.getBytes();
  }

}
