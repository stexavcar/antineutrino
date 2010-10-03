package org.neutrino.plankton;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.neutrino.plankton.PMap.Thunk;
import org.neutrino.plankton.annotations.Factory;
import org.neutrino.plankton.annotations.Growable;
import org.neutrino.plankton.annotations.SeedMember;

/**
 * A registry where plankton seed types can be registered.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class PlanktonRegistry {

  public interface ICodec<T> {
    public PValue encode(Plankton plankton, T obj);
    public T decode(Plankton plankton, PValue payload);
    public Class<?> getSubjectClass();
  }

  private class AnnotationCodec<T> implements ICodec<T> {

    private final Class<T> klass;

    public AnnotationCodec(Class<T> klass) {
      this.klass = klass;
    }

    @SuppressWarnings("unchecked")
    private T createNewInstance(PValue payload) throws InvocationTargetException, InstantiationException, IllegalAccessException {
      try {
        return klass.newInstance();
      } catch (InstantiationException ie) {
        for (Method method : klass.getDeclaredMethods()) {
          if (method.getAnnotation(Factory.class) != null) {
            return (T) method.invoke(null, payload);
          }
        }
        return null;
      }
    }

    @Override
    public T decode(Plankton plankton, PValue payload) {
      PMap fields = (PMap) payload;
      try {
        T result = createNewInstance(payload);
        for (Field field : klass.getDeclaredFields()) {
          SeedMember seedMember = field.getAnnotation(SeedMember.class);
          if (seedMember != null) {
            String name = seedMember.value();
            if (name.isEmpty())
              name = field.getName();
            PValue value = fields.get(Plankton.newString(name), null);
            Object rawValue = decodeValue(plankton, value);
            field.set(result, rawValue);
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

    @Override
    public PValue encode(Plankton plankton, T obj) {
      return encodeSeedPayload(plankton, obj);
    }

    public Class<T> getSubjectClass() {
      return klass;
    }

  }

  PMap encodeSeedPayload(Plankton plankton, Object obj) {
    Class<?> klass = obj.getClass();
    Map<PString, PValue> members = new HashMap<PString, PValue>();
    for (Field field : klass.getDeclaredFields()) {
      SeedMember seedMember = field.getAnnotation(SeedMember.class);
      if (seedMember != null) {
        try {
          String name = seedMember.value();
          if (name.isEmpty())
            name = field.getName();
          Object rawValue = field.get(obj);
          PValue value = encodeValue(plankton, rawValue);
          members.put(Plankton.newString(name), value);
        } catch (IllegalArgumentException e) {
          throw new RuntimeException(e);
        } catch (IllegalAccessException e) {
          throw new RuntimeException(e);
        }
      }
    }
    return Plankton.newMap(members);
  }

  @SuppressWarnings("unchecked")
  private <T> PValue encode(ICodec<T> codec, Plankton plankton, Object value) {
    return codec.encode(plankton, (T) value);
  }

  private final Map<String, ICodec<?>> seedCodecs = new HashMap<String, ICodec<?>>();

  private final Map<Class<?>, ICodec<?>> typeCodecs = new HashMap<Class<?>, ICodec<?>>() {{
    put(INTEGER_CODEC.getSubjectClass(), INTEGER_CODEC);
    put(STRING_CODEC.getSubjectClass(), STRING_CODEC);
    put(BLOB_CODEC.getSubjectClass(), BLOB_CODEC);
  }};

  @SuppressWarnings("unchecked")
  private final List<? extends ICodec<?>> genericCodecs = Arrays.asList(
    MAP_CODEC, LIST_CODEC
  );

  public PlanktonRegistry() {

  }

  public <T> void register(Class<T> klass) {
    Growable seedable = klass.getAnnotation(Growable.class);
    String tag = seedable.value();
    seedCodecs.put(tag, new AnnotationCodec<T>(klass));
  }

  public ICodec<?> getSeedCodec(String tag) {
    return seedCodecs.get(tag);
  }

  public PValue encodeValue(Plankton plankton, Object value) {
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

  public Object decodeValue(Plankton plankton, PValue value) {
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
    default:
      assert false;
      return null;
    }
  }

  private static final ICodec<Integer> INTEGER_CODEC = new ICodec<Integer>() {
    @Override
    public Integer decode(Plankton plankton, PValue payload) {
      return ((PInteger) payload).getValue();
    }
    @Override
    public PValue encode(Plankton plankton, Integer obj) {
      return Plankton.newInteger(obj);
    }
    @Override
    public Class<Integer> getSubjectClass() {
      return Integer.class;
    }
  };

  private static final ICodec<String> STRING_CODEC = new ICodec<String>() {
    @Override
    public String decode(Plankton plankton, PValue payload) {
      return ((PString) payload).getValue();
    }
    @Override
    public PValue encode(Plankton plankton, String obj) {
      return Plankton.newString(obj);
    }
    @Override
    public Class<String> getSubjectClass() {
      return String.class;
    }
  };

  private static final ICodec<byte[]> BLOB_CODEC = new ICodec<byte[]>() {
    @Override
    public byte[] decode(Plankton plankton, PValue payload) {
      return ((PBlob) payload).getData();
    }
    @Override
    public PValue encode(Plankton plankton, byte[] obj) {
      return Plankton.newBlob(obj);
    }
    @Override
    public Class<?> getSubjectClass() {
      return byte[].class;
    }
  };

  private static final ICodec<Map<?, ?>> MAP_CODEC = new ICodec<Map<?, ?>>() {
    @Override
    public Map<?, ?> decode(final Plankton plankton, PValue payload) {
      PMap map = (PMap) payload;
      final Map<Object, Object> result = new HashMap<Object, Object>();
      map.forEach(new Thunk() {
        @Override
        public boolean call(PValue key, PValue value) {
          Object from = plankton.getRegistry().decodeValue(plankton, key);
          Object to = plankton.getRegistry().decodeValue(plankton, value);
          result.put(from, to);
          return true;
        }
      });
      return result;
    }
    @Override
    public PValue encode(Plankton plankton, Map<?, ?> obj) {
      Map<PValue, PValue> map = new HashMap<PValue, PValue>();
      for (Map.Entry<?, ?> entry : obj.entrySet()) {
        PValue key = plankton.getRegistry().encodeValue(plankton, entry.getKey());
        PValue value = plankton.getRegistry().encodeValue(plankton, entry.getValue());
        map.put(key, value);
      }
      return Plankton.newMap(map);
    }
    @Override
    public Class<?> getSubjectClass() {
      return Map.class;
    }
  };

  private static final ICodec<List<?>> LIST_CODEC = new ICodec<List<?>>() {
    @Override
    public List<?> decode(Plankton plankton, PValue value) {
      PArray array = (PArray) value;
      List<Object> elms = new ArrayList<Object>();
      for (int i = 0; i < array.length(); i++) {
        PValue elm = array.get(i);
        elms.add(plankton.getRegistry().decodeValue(plankton, elm));
      }
      return elms;
    }
    @Override
    public PValue encode(Plankton plankton, List<?> list) {
      List<PValue> values = new ArrayList<PValue>();
      for (Object obj : list) {
        values.add(plankton.getRegistry().encodeValue(plankton, obj));
      }
      return Plankton.newArray(values);
    }
    @Override
    public Class<?> getSubjectClass() {
      return List.class;
    }
  };

}
