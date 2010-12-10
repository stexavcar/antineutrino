package org.neutrino.plankton;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.PlanktonEncoder.ObjectEncoder;

public class PlanktonDecoder {

  private abstract class Template<T> {

    /**
     * Called when the beginning of the template is encountered but
     * before the payload has been read.  If an object is returned this
     * will be the one bound to the object's index.
     */
    public abstract T onTemplateStart() throws IOException;

    /**
     * Called after the payload has been parsed.  If {@link #onTemplateStart()}
     * returned null and this returns a value then that value will be
     * bound to this object's index.
     */
    public abstract T onTemplatePayload(T obj) throws IOException;


  }

  private static class TemplateScope {

    private final TemplateScope parent;
    private final int start;
    private final List<Template<?>> templates;

    public TemplateScope(TemplateScope parent, int start) {
      this.parent = parent;
      this.start = start;
      this.templates = new ArrayList<Template<?>>();
    }

    public Template<?> getTemplate(int index) {
      int localIndex = index - start;
      if (0 <= localIndex && localIndex < templates.size())
        return templates.get(localIndex);
      return parent.getTemplate(index);
    }

  }

  private TemplateScope templates = null;
  private final Map<Integer, Object> refs = new HashMap<Integer, Object>();
  private final LowLevelDecoder in;
  private final ClassIndex classes;
  private int nextIndex = 0;

  public PlanktonDecoder(ClassIndex classes, InputStream in) {
    this.classes = classes;
    this.in = new LowLevelDecoder(in);
  }

  private int getCurrentIndex() {
    return nextIndex - 1;
  }

  public Object read() throws IOException {
    return read(in.read());
  }

  public Object read(int tag) throws IOException {
    switch (tag) {
    case PlanktonEncoder.kIntTag:
      return (int) in.readSigned();
    case PlanktonEncoder.kStringTag:
      return parseString();
    case PlanktonEncoder.kNullTag:
      return null;
    case PlanktonEncoder.kTrueTag:
      return true;
    case PlanktonEncoder.kFalseTag:
      return false;
    case PlanktonEncoder.kMapTag:
      nextIndex++;
      return parseMap();
    case PlanktonEncoder.kArrayTag:
      nextIndex++;
      return parseArray();
    case PlanktonEncoder.kRefTag:
      return parseReference();
    case PlanktonEncoder.kScopeTag:
      return parseTemplateScope();
    case PlanktonEncoder.kTemplateTag:
      nextIndex++;
      return parseTemplateInstance();
    case PlanktonEncoder.kBlobTag:
      return parseBlob();
    default:
      throw new AssertionError("Unknown tag: " + tag);
    }
  }

  private Object parseTemplateScope() throws IOException {
    int start = (int) in.readUnsigned();
    int count = (int) in.readUnsigned();
    TemplateScope oldScope = templates;
    TemplateScope newScope = new TemplateScope(oldScope, start);
    templates = newScope;
    Object result;
    try {
      for (int i = 0; i < count; i++) {
        Template<?> template = readTemplate();
        newScope.templates.add(template);
      }
      result = read();
    } finally {
      templates = oldScope;
    }
    return result;
  }

  private <T> T instantiateTemplate(Template<T> template, boolean storeRef) throws IOException {
    int index = getCurrentIndex();
    T beforeInstance = template.onTemplateStart();
    if (storeRef && beforeInstance != null)
      refs.put(index, beforeInstance);
    T afterInstance = template.onTemplatePayload(beforeInstance);
    if (storeRef && afterInstance != null) {
      refs.put(index, afterInstance);
      beforeInstance = afterInstance;
    }
    return beforeInstance;
  }

  private Object parseTemplateInstance() throws IOException {
    int index = (int) in.readUnsigned();
    Template<?> template = templates.getTemplate(index);
    return instantiateTemplate(template, true);
  }

  private Object parseReference() throws IOException {
    int index = (int) in.readUnsigned();
    return refs.get(index);
  }

  private String parseString() throws IOException {
    return new String(parseBlob());
  }

  private byte[] parseBlob() throws IOException {
    int length = (int) in.readUnsigned();
    byte[] bytes = new byte[length];
    for (int i = 0; i < length; i++)
      bytes[i] = (byte) in.read();
    return bytes;
  }

  private List<?> parseArray() throws IOException {
    int length = (int) in.readUnsigned();
    ArrayList<Object> result = new ArrayList<Object>(length);
    refs.put(getCurrentIndex(), result);
    for (int i = 0; i < length; i++)
      result.add(read());
    return result;
  }

  private Template<?> parseArrayTemplate() throws IOException {
    final int length = (int) in.readUnsigned();
    final ArrayList<Template<?>> parts = new ArrayList<Template<?>>();
    for (int i = 0; i < length; i++)
      parts.add(readTemplate());
    return new Template<List<Object>>() {
      @Override
      public List<Object> onTemplateStart() {
        return new ArrayList<Object>();
      }
      @Override
      public List<Object> onTemplatePayload(List<Object> value) throws IOException {
        for (int i = 0; i < length; i++)
          value.add(instantiateTemplate(parts.get(i), false));
        return null;
      }
    };
  }

  private Map<?, ?> parseMap() throws IOException {
    int length = (int) in.readUnsigned();
    HashMap<Object, Object> result = new HashMap<Object, Object>();
    refs.put(getCurrentIndex(), result);
    for (int i = 0; i < length; i++) {
      Object key = read();
      Object value = read();
      result.put(key, value);
    }
    return result;
  }

  private Template<?> parseMapTemplate() throws IOException {
    final int length = (int) in.readUnsigned();
    final List<Template<?>> parts = new ArrayList<Template<?>>();
    for (int i = 0; i < length; i++) {
      parts.add(readTemplate());
      parts.add(readTemplate());
    }
    return new Template<Map<Object, Object>>() {
      @Override
      public Map<Object, Object> onTemplateStart() {
        return new HashMap<Object, Object>();
      }
      @Override
      public Map<Object, Object> onTemplatePayload(Map<Object, Object> map) throws IOException {
        for (int i = 0; i < length; i++) {
          Object key = instantiateTemplate(parts.get(2 * i), false);
          Object value = instantiateTemplate(parts.get(2 * i + 1), false);
          map.put(key, value);
        }
        return null;
      }
    };
  }

  private Template<?> parseObjectTemplate() throws IOException {
    String tag = (String) read();
    final Template<?> payloadTemplate = readTemplate();
    final ObjectEncoder codec = classes.getEncoder(tag);
    return new Template<Object>() {
      @Override
      public Object onTemplateStart() {
        return codec.isAtomic() ? null : codec.newInstance();
      }
      @Override
      public Object onTemplatePayload(Object obj) throws IOException {
        Map<String, Object> fields = (Map<String, Object>) instantiateTemplate(payloadTemplate, false);
        if (codec.isAtomic()) {
          Object arg = fields.values().iterator().next();
          return codec.newAtomicInstance(arg);
        } else {
          for (Map.Entry<String, Object> entry : fields.entrySet())
            codec.set(obj, entry.getKey(), entry.getValue());
          return null;
        }
      }
    };
  }

  private Template<?> parseConstantTemplate(int tag) throws IOException {
    final Object value = read(tag);
    return new Template<Object>() {
      @Override
      public Object onTemplateStart() {
        return value;
      }
      @Override
      public Object onTemplatePayload(Object obj) throws IOException {
        return null;
      }
    };
  }

  private Template<?> parsePlaceholderTemplate() throws IOException {
    return new Template<Object>() {
      @Override
      public Object onTemplateStart() throws IOException {
        return PlanktonDecoder.this.read();
      }
      @Override
      public Object onTemplatePayload(Object obj) {
        return null;
      }
    };
  }

  private Template<?> readTemplate() throws IOException {
    int tag = in.read();
    switch (tag) {
    case PlanktonEncoder.kArrayTag:
      return parseArrayTemplate();
    case PlanktonEncoder.kMapTag:
      return parseMapTemplate();
    case PlanktonEncoder.kPlaceholderTag:
      return parsePlaceholderTemplate();
    case PlanktonEncoder.kObjectTag:
      return parseObjectTemplate();
    default:
      return parseConstantTemplate(tag);
    }
  }

}
