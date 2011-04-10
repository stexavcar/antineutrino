package org.neutrino.plankton;

import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.IdentityHashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.PlanktonDecoder.Template;
import org.neutrino.runtime.RValue;

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
    public EncoderRecord(int index) {
      this.index = index;
    }
  }

  public static interface IObjectEncoder {

    public void writeTemplate(PlanktonEncoder encoder) throws IOException;

    public boolean canHandle(Object obj);

    public void writeObject(Object obj, PlanktonEncoder encoder) throws IOException;

    public Object onTemplateStart();

    public Object onTemplatePayload(Template<?> template, Object obj,
        PlanktonDecoder decoder) throws IOException;

  }

  public static class EnumEncoder implements IObjectEncoder {

    private final Class<? extends Enum<?>> klass;

    public EnumEncoder(Class<? extends Enum<?>> klass) {
      this.klass = klass;
    }

    @Override
    public boolean canHandle(Object obj) {
      return klass.isInstance(obj);
    }

    @Override
    public Object onTemplateStart() {
      return null;
    }

    @Override
    public Object onTemplatePayload(Template<?> template, Object obj,
        PlanktonDecoder decoder) throws IOException {
      int index = (Integer) decoder.instantiateTemplate(template, false);
      return klass.getEnumConstants()[index];
    }

    @Override
    public void writeTemplate(PlanktonEncoder encoder) throws IOException {
      encoder.out.write(kObjectTag);
      encoder.writeString(klass.getName());
      encoder.out.write(kPlaceholderTag);
    }

    @Override
    public void writeObject(Object obj, PlanktonEncoder encoder)
        throws IOException {
      encoder.internalWrite(klass.cast(obj).ordinal());
    }

  }

  public static class PojoEncoder implements IObjectEncoder {

    private final Class<?> klass;
    private final Map<String, Field> fieldMap = new HashMap<String, Field>();
    private final List<Field> fields = new ArrayList<Field>();
    private final Method atomicFactory;

    public PojoEncoder(Class<?> klass) {
      assert !klass.isEnum();
      this.klass = klass;
      for (Field field : klass.getDeclaredFields()) {
        if (field.getAnnotation(Store.class) != null) {
          fieldMap.put(field.getName(), field);
          fields.add(field);
        }
      }
      boolean isAtomic = (klass.getAnnotation(Atomic.class) != null);
      if (isAtomic) {
        for (Method method : klass.getDeclaredMethods()) {
          if (method.getAnnotation(Generator.class) != null) {
            atomicFactory = method;
            return;
          }
        }
        atomicFactory = null;
      } else {
        atomicFactory = null;
      }
    }

    public boolean canHandle(Object obj) {
      return klass.isInstance(obj);
    }

    public boolean isAtomic() {
      return atomicFactory != null;
    }

    public Object newAtomicInstance(Object... args) {
      try {
        return atomicFactory.invoke(null, args);
      } catch (IllegalArgumentException e) {
        throw new RuntimeException(e);
      } catch (IllegalAccessException e) {
        throw new RuntimeException(e);
      } catch (InvocationTargetException e) {
        throw new RuntimeException(e);
      }
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

    @Override
    public void writeTemplate(PlanktonEncoder encoder) throws IOException {
      encoder.out.write(kObjectTag);
      encoder.writeString(klass.getName());
      encoder.out.write(kMapTag);
      encoder.out.writeUnsigned(fields.size());
      for (Field field : fields) {
        encoder.writeString(field.getName());
        encoder.out.write(kPlaceholderTag);
      }
    }

    @Override
    public void writeObject(Object obj, PlanktonEncoder encoder) throws IOException {
      for (Field field : fields) {
        Object value;
        try {
          value = field.get(obj);
        } catch (IllegalArgumentException e) {
          throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
          throw new RuntimeException(e);
        }
        encoder.internalWrite(value);
      }
    }

    @Override
    public Object onTemplateStart() {
      return isAtomic() ? null : newInstance();
    }

    @Override
    public Object onTemplatePayload(Template<?> template, Object obj, PlanktonDecoder decoder)
        throws IOException {
      @SuppressWarnings("unchecked")
      Map<String, Object> fields = (Map<String, Object>) decoder.instantiateTemplate(template, false);
      if (isAtomic()) {
        if (fields.isEmpty()) {
          return newAtomicInstance();
        } else {
          Object arg = fields.values().iterator().next();
          return newAtomicInstance(arg);
        }
      } else {
        for (Map.Entry<String, Object> entry : fields.entrySet())
          set(obj, entry.getKey(), entry.getValue());
        return null;
      }
    }
  }

  public static class BuiltinObjectEncoder implements IObjectEncoder {

    public static final String ID = "built-in";
    private final IBuiltinObjectIndex builtins;

    public BuiltinObjectEncoder(IBuiltinObjectIndex builtins) {
      this.builtins = builtins;
    }

    @Override
    public void writeTemplate(PlanktonEncoder encoder) throws IOException {
      encoder.out.write(kObjectTag);
      encoder.writeString(ID);
      encoder.out.write(kPlaceholderTag);
    }

    @Override
    public void writeObject(Object obj, PlanktonEncoder encoder)
        throws IOException {
      encoder.internalWrite(builtins.getKey((RValue) obj));
    }

    @Override
    public boolean canHandle(Object obj) {
      return obj instanceof RValue && builtins.getKey((RValue) obj) != null;
    }

    @Override
    public Object onTemplatePayload(Template<?> template, Object obj,
        PlanktonDecoder decoder) throws IOException {
      Object key = decoder.instantiateTemplate(template, false);
      return builtins.getValue(key);
    }

    @Override
    public Object onTemplateStart() {
      return null;
    }

  }

  private final Map<Object, EncoderRecord> seen = new IdentityHashMap<Object, EncoderRecord>();
  private final LowLevelEncoder out;
  private final ClassIndex klasses;
  private int nextIndex = 0;

  public PlanktonEncoder(ClassIndex klasses, IBuiltinObjectIndex builtins) {
    this.out = new LowLevelEncoder();
    this.klasses = new ClassIndex();
    this.klasses.addEncoder(BuiltinObjectEncoder.ID, new BuiltinObjectEncoder(builtins));
    this.klasses.addAll(klasses);
  }

  public void write(Object obj) throws IOException {
    writeTemplates(klasses.getEncoders());
    internalWrite(obj);
    out.flush();
  }


  private void writeTemplates(List<IObjectEncoder> encoders) throws IOException {
    out.write(kScopeTag);
    out.writeUnsigned(0);
    out.writeUnsigned(encoders.size());
    for (IObjectEncoder encoder : encoders) {
      encoder.writeTemplate(this);
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
    } else if (obj instanceof Boolean) {
      this.writeBoolean((Boolean) obj);
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
      writeObject(obj);
    }
  }

  private void writeObject(Object obj) throws IOException {
    int index = 0;
    for (IObjectEncoder encoder : klasses.getEncoders()) {
      if (encoder.canHandle(obj)) {
        nextIndex++;
        this.writeObject(encoder, obj, index);
        return;
      }
      index++;
    }
    assert false : obj.getClass();
  }

  public void writeObject(IObjectEncoder encoder, Object obj, int index) throws IOException {
    out.write(kTemplateTag);
    int objIndex = getCurrentIndex();
    seen.put(obj, new EncoderRecord(objIndex));
    out.writeUnsigned(index);
    encoder.writeObject(obj, this);
  }

  private void writeReference(EncoderRecord record) throws IOException {
    out.write(kRefTag);
    out.writeUnsigned(record.index);
  }

  private void writeList(List<?> obj) throws IOException {
    out.write(kArrayTag);
    int index = getCurrentIndex();
    seen.put(obj, new EncoderRecord(index));
    out.writeUnsigned(obj.size());
    for (Object elm : obj)
      internalWrite(elm);
  }

  private void writeMap(Map<?, ?> obj) throws IOException {
    out.write(kMapTag);
    int index = getCurrentIndex();
    seen.put(obj, new EncoderRecord(index));
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

  private void writeBoolean(Boolean obj) throws IOException {
    if (obj.booleanValue()) {
      out.write(kTrueTag);
    } else {
      out.write(kFalseTag);
    }
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
