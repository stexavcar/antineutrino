package org.neutrino.oldton;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.oldton.PMap.Thunk;
import org.neutrino.oldton.annotations.Factory;
import org.neutrino.oldton.annotations.Growable;
import org.neutrino.oldton.annotations.SeedMember;

/**
 * A registry where plankton seed types can be registered.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class OldtonRegistry {

  public interface ICodec<T> {
    public PValue encode(Oldton plankton, T obj);
    public T decode(Oldton plankton, PValue payload);
    public Class<?> getSubjectClass();
  }

  private class AnnotationCodec<T> implements ICodec<T> {

    private final Class<T> klass;
    private boolean isInitialized = false;

    public AnnotationCodec(Class<T> klass) {
      this.klass = klass;
    }

    @SuppressWarnings("unchecked")
    private T createNewInstance(PValue payload) throws InvocationTargetException, InstantiationException, IllegalAccessException {
      try {
        return klass.newInstance();
      } catch (InstantiationException ie) {
        for (Method method : klass.getDeclaredMethods()) {
          Factory factory = method.getAnnotation(Factory.class);
          if (factory != null) {
            isInitialized = !factory.initialize();
            return (T) method.invoke(null, payload);
          }
        }
        return null;
      }
    }

    public T decode(Oldton plankton, PValue payload) {
      PMap fields = (PMap) payload;
      try {
        T result = createNewInstance(payload);
        if (!isInitialized) {
          for (Field field : klass.getDeclaredFields()) {
            SeedMember seedMember = field.getAnnotation(SeedMember.class);
            if (seedMember != null) {
              String name = seedMember.value();
              if (name.length() == 0)
                name = field.getName();
              PValue value = fields.get(Oldton.newString(name), null);
              Object rawValue = decodeValue(plankton, value);
              field.set(result, rawValue);
            }
          }
        }
        return result;
      } catch (InstantiationException e) {
        throw new RuntimeException(e);
      } catch (IllegalAccessException e) {
        throw new RuntimeException(e);
      } catch (InvocationTargetException e) {
        throw new RuntimeException(e);
      }
    }

    public PValue encode(Oldton plankton, T obj) {
      return encodeSeedPayload(plankton, obj);
    }

    public Class<T> getSubjectClass() {
      return klass;
    }

  }

  PMap encodeSeedPayload(Oldton plankton, Object obj) {
    Class<?> klass = obj.getClass();
    Map<PString, PValue> members = new HashMap<PString, PValue>();
    for (Field field : klass.getDeclaredFields()) {
      SeedMember seedMember = field.getAnnotation(SeedMember.class);
      if (seedMember != null) {
        try {
          String name = seedMember.value();
          if (name.length() == 0)
            name = field.getName();
          Object rawValue = field.get(obj);
          PValue value = encodeValue(plankton, rawValue);
          members.put(Oldton.newString(name), value);
        } catch (IllegalArgumentException e) {
          throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
          throw new RuntimeException(e);
        }
      }
    }
    return Oldton.newMap(members);
  }

  @SuppressWarnings("unchecked")
  private <T> PValue encode(ICodec<T> codec, Oldton plankton, Object value) {
    return codec.encode(plankton, (T) value);
  }

  private final Map<String, ICodec<?>> seedCodecs = new HashMap<String, ICodec<?>>();

  private final Map<Class<?>, ICodec<?>> typeCodecs = new HashMap<Class<?>, ICodec<?>>() {{
    put(INTEGER_CODEC.getSubjectClass(), INTEGER_CODEC);
    put(STRING_CODEC.getSubjectClass(), STRING_CODEC);
    put(BOOL_CODEC.getSubjectClass(), BOOL_CODEC);
    put(BLOB_CODEC.getSubjectClass(), BLOB_CODEC);
  }};

  @SuppressWarnings("unchecked")
  private final List<? extends ICodec<?>> genericCodecs = Arrays.asList(
    MAP_CODEC, LIST_CODEC
  );

  public OldtonRegistry() {

  }

  public <T> void register(Class<T> klass) {
    Growable seedable = klass.getAnnotation(Growable.class);
    String tag = seedable.value();
    seedCodecs.put(tag, new AnnotationCodec<T>(klass));
  }

  public ICodec<?> getSeedCodec(String tag) {
    return seedCodecs.get(tag);
  }

  public PValue encodeValue(Oldton plankton, Object value) {
    Class<?> type = value.getClass();
    if (value instanceof ISeedable) {
      return plankton.newSeed((ISeedable) value);
    } else if (typeCodecs.containsKey(type)) {
      ICodec<?> codec = typeCodecs.get(type);
      return encode(codec, plankton, value);
    } else {
      for (ICodec<?> codec : genericCodecs) {
        if (codec.getSubjectClass().isAssignableFrom(type)) {
          return encode(codec, plankton, value);
        }
      }
      assert false;
      return null;
    }
  }

  public Object decodeValue(Oldton plankton, PValue value) {
    switch (value.getType()) {
    case SEED:
      return ((PSeed) value).grow(Object.class);
    case INTEGER:
      return INTEGER_CODEC.decode(plankton, value);
    case BLOB:
      return BLOB_CODEC.decode(plankton, value);
    case STRING:
      return STRING_CODEC.decode(plankton, value);
    case MAP:
      return MAP_CODEC.decode(plankton, value);
    case ARRAY:
      return LIST_CODEC.decode(plankton, value);
    case BOOL:
      return BOOL_CODEC.decode(plankton, value);
    default:
      assert false;
      return null;
    }
  }

  private static final ICodec<Integer> INTEGER_CODEC = new ICodec<Integer>() {
    public Integer decode(Oldton plankton, PValue payload) {
      return ((PInteger) payload).getValue();
    }
    public PValue encode(Oldton plankton, Integer obj) {
      return Oldton.newInteger(obj);
    }
    public Class<Integer> getSubjectClass() {
      return Integer.class;
    }
  };

  private static final ICodec<Boolean> BOOL_CODEC = new ICodec<Boolean>() {
    public Boolean decode(Oldton plankton, PValue payload) {
      return ((PBool) payload).getValue();
    }
    public PValue encode(Oldton plankton, Boolean obj) {
      return Oldton.newBool(obj);
    }
    public Class<Boolean> getSubjectClass() {
      return Boolean.class;
    }
  };

  private static final ICodec<String> STRING_CODEC = new ICodec<String>() {
    public String decode(Oldton plankton, PValue payload) {
      return ((PString) payload).getValue();
    }
    public PValue encode(Oldton plankton, String obj) {
      return Oldton.newString(obj);
    }
    public Class<String> getSubjectClass() {
      return String.class;
    }
  };

  private static final ICodec<byte[]> BLOB_CODEC = new ICodec<byte[]>() {
    public byte[] decode(Oldton plankton, PValue payload) {
      return ((PBlob) payload).getData();
    }
    public PValue encode(Oldton plankton, byte[] obj) {
      return Oldton.newBlob(obj);
    }
    public Class<?> getSubjectClass() {
      return byte[].class;
    }
  };

  private static final ICodec<Map<?, ?>> MAP_CODEC = new ICodec<Map<?, ?>>() {
    public Map<?, ?> decode(final Oldton plankton, PValue payload) {
      PMap map = (PMap) payload;
      final Map<Object, Object> result = new HashMap<Object, Object>();
      map.forEach(new Thunk() {
        public boolean call(PValue key, PValue value) {
          Object from = plankton.getRegistry().decodeValue(plankton, key);
          Object to = plankton.getRegistry().decodeValue(plankton, value);
          result.put(from, to);
          return true;
        }
      });
      return result;
    }
    public PValue encode(Oldton plankton, Map<?, ?> obj) {
      Map<PValue, PValue> map = new HashMap<PValue, PValue>();
      for (Map.Entry<?, ?> entry : obj.entrySet()) {
        PValue key = plankton.getRegistry().encodeValue(plankton, entry.getKey());
        PValue value = plankton.getRegistry().encodeValue(plankton, entry.getValue());
        map.put(key, value);
      }
      return Oldton.newMap(map);
    }
    public Class<?> getSubjectClass() {
      return Map.class;
    }
  };

  private static final ICodec<List<?>> LIST_CODEC = new ICodec<List<?>>() {
    public List<?> decode(Oldton plankton, PValue value) {
      PArray array = (PArray) value;
      List<Object> elms = new ArrayList<Object>();
      for (int i = 0; i < array.length(); i++) {
        PValue elm = array.get(i);
        elms.add(plankton.getRegistry().decodeValue(plankton, elm));
      }
      return elms;
    }
    public PValue encode(Oldton plankton, List<?> list) {
      List<PValue> values = new ArrayList<PValue>();
      for (Object obj : list) {
        values.add(plankton.getRegistry().encodeValue(plankton, obj));
      }
      return Oldton.newArray(values);
    }
    public Class<?> getSubjectClass() {
      return List.class;
    }
  };

}
