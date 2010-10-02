package org.neutrino.plankton;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.neutrino.plankton.PMap.ThrowingThunk;
import org.neutrino.plankton.PMap.Thunk;
import org.neutrino.plankton.annotations.Growable;

public class Plankton {

  private final PlanktonRegistry registry;

  public Plankton(PlanktonRegistry registry) {
    this.registry = registry;
  }

  private static abstract class ValueImpl implements PValue {

    @Override
    public String toString() {
      return Plankton.toString(this);
    }

  }

  private static class MapImpl extends ValueImpl implements PMap {

    private final Map<? extends PValue, ? extends PValue> values;

    public MapImpl(Map<? extends PValue, ? extends PValue> values) {
      this.values = values;
    }

    @Override
    public <E extends Throwable> void forEach(ThrowingThunk<E> thunk) throws E {
      for (Map.Entry<? extends PValue, ? extends PValue> entry : values.entrySet()) {
        PValue key = entry.getKey();
        PValue value = entry.getValue();
        if (!thunk.call(key, value))
          return;
      }
    }

    @Override
    public PValue get(PValue key, PValue ifMissing) {
      return values.containsKey(key) ? values.get(key) : ifMissing;
    }

    @Override
    public int size() {
      return values.size();
    }

    @Override
    public Type getType() {
      return Type.MAP;
    }

  }

  private static class StringImpl extends ValueImpl implements PString {

    private final String str;

    public StringImpl(String str) {
      this.str = str;
    }

    @Override
    public String getValue() {
      return this.str;
    }

    @Override
    public Type getType() {
      return Type.STRING;
    }

    @Override
    public int hashCode() {
      return str.hashCode();
    }

    @Override
    public boolean equals(Object that) {
      return (that instanceof PString) && ((PString) that).getValue().equals(str);
    }

  }

  private static class BlobImpl extends ValueImpl implements PBlob {

    private final byte[] data;

    public BlobImpl(byte[] data) {
      this.data = data;
    }

    @Override
    public Type getType() {
      return Type.BLOB;
    }

    @Override
    public int length() {
      return data.length;
    }

    @Override
    public byte[] getData() {
      return this.data;
    }

    @Override
    public int get(int offset) {
      return data[offset];
    }

  }

  private static class IntegerImpl extends ValueImpl implements PInteger {

    private final int value;

    public IntegerImpl(int value) {
      this.value = value;
    }

    @Override
    public int getValue() {
      return value;
    }

    @Override
    public Type getType() {
      return Type.INTEGER;
    }

    @Override
    public int hashCode() {
      return value;
    }

    @Override
    public boolean equals(Object that) {
      return (that instanceof PInteger) && ((PInteger) that).getValue() == value;
    }

  }

  private static class ArrayImpl extends ValueImpl implements PArray {

    private final List<? extends PValue> values;

    private ArrayImpl(List<? extends PValue> values) {
      this.values = values;
    }

    @Override
    public PValue get(int index) {
      return values.get(index);
    }

    @Override
    public int length() {
      return values.size();
    }

    @Override
    public Type getType() {
      return Type.ARRAY;
    }

  }

  private class SeedImpl extends ValueImpl implements PSeed {

    private final PValue tag;
    private final PValue payload;
    private Object value;

    public SeedImpl(PValue tag, PValue payload) {
      this.tag = tag;
      this.payload = payload;
    }

    @Override
    public PValue getPayload() {
      return payload;
    }

    @Override
    public PValue getTag() {
      return tag;
    }

    @Override
    public Type getType() {
      return Type.SEED;
    }

    @Override
    public <T> T grow(Class<T> klass) {
      if (this.value == null) {
        value = Plankton.this.growSeed(this);
      }
      if (value == null) {
        return null;
      } else {
        return klass.cast(value);
      }
    }

  }

  public static PMap newMap(Map<? extends PValue, ? extends PValue> elms) {
    return new MapImpl(elms);
  }

  public static PString newString(String str) {
    return new StringImpl(str);
  }

  public static PBlob newBlob(byte[] data) {
    return new BlobImpl(data);
  }

  public static PInteger newInteger(int value) {
    return new IntegerImpl(value);
  }

  public static PArray newArray(Collection<? extends PValue> elms) {
    return new ArrayImpl(new ArrayList<PValue>(elms));
  }

  public static PArray newArray(PValue... elms) {
    return new ArrayImpl(Arrays.asList(elms));
  }

  public PSeed newSeed(String tag, PValue payload) {
    return new SeedImpl(newString(tag), payload);
  }

  public PSeed newSeed(ISeedable value) {
    Growable growable = value.getClass().getAnnotation(Growable.class);
    String tag = growable.value();
    return newSeed(tag, registry.encodeSeedPayload(this, value));
  }

  private Object growSeed(PSeed seed) {
    String tag = ((PString) seed.getTag()).getValue();
    PlanktonRegistry.ICodec<?> codec = registry.getSeedCodec(tag);
    if (codec == null)
      return null;
    return codec.decode(this, seed.getPayload());
  }

  PlanktonRegistry getRegistry() {
    return registry;
  }

  /**
   * Converts the value into a human-readable string.
   */
  public static String toString(PValue value) {
    switch (value.getType()) {
    case MAP: {
      final Map<String, String> elms = new TreeMap<String, String>();
      ((PMap) value).forEach(new Thunk() {
        @Override
        public boolean call(PValue key, PValue value) {
          elms.put(Plankton.toString(key), Plankton.toString(value));
          return true;
        }
      });
      return elms.toString();
    }
    case ARRAY: {
      PArray array = (PArray) value;
      List<String> elms = new ArrayList<String>();
      int length = array.length();
      for (int i = 0; i < length; i++)
        elms.add(toString(array.get(i)));
      return elms.toString();
    }
    case STRING:
      return "\"" + ((PString) value).getValue() + "\"";
    case BLOB: {
      PBlob blob = (PBlob) value;
      StringBuilder buf = new StringBuilder();
      buf.append("#[");
      int length = blob.length();
      for (int i = 0; i < length; i++) {
        if (i > 0)
          buf.append(" ");
        int b = blob.get(i);
        String str = Integer.toHexString(b);
        if (str.length() < 2)
          buf.append("0");
        buf.append(str);
      }
      buf.append("]");
      return buf.toString();
    }
    case INTEGER:
      return Integer.toString(((PInteger) value).getValue());
    case SEED:
      PSeed seed = (PSeed) value;
      Object obj = seed.grow(Object.class);
      if (obj == null) {
        return "#<" + seed.getTag() + " " + toString(seed.getPayload()) + ">";
      } else {
        return obj.toString();
      }
    }
    return null;
  }

  private static class CodecData {

    public static final int kNoReference = -1;
    public static final boolean kUseReferences = true;

    private final Map<Integer, Object> objs = new HashMap<Integer, Object>();
    private final Map<Object, Integer> indices = new HashMap<Object, Integer>();

    public int getIndex(Object obj) {
      Integer result = indices.get(obj);
      return (result == null) ? kNoReference : result;
    }

    public void map(int index, Object obj) {
      objs.put(index, obj);
      indices.put(obj, index);
    }

    public Object getObject(int index) {
      return objs.get(index);
    }

    public int register(Object obj) {
      int index = objs.size();
      map(index, obj);
      return index;
    }

  }

  static final int kStringTag = 0;
  static final int kMapTag = 1;
  static final int kBlobTag = 2;
  static final int kIntegerTag = 3;
  static final int kArrayTag = 4;
  static final int kSeedTag = 5;
  static final int kNewReference = 6;
  static final int kGetReference = 7;

  public void write(OutputStream out, PValue value) throws IOException {
    write(out, value, new CodecData());
  }

  private void write(final OutputStream out, PValue value,
      final CodecData data) throws IOException {
    PValue.Type type = value.getType();
    switch (type) {
    case STRING: {
      String str = ((PString) value).getValue();
      if (str.length() < 4 || !CodecData.kUseReferences) {
        out.write(type.getTag());
        writeString(out, str);
      } else {
        int index = data.getIndex(str);
        if (index == CodecData.kNoReference) {
          int newIndex = data.register(str);
          out.write(kNewReference);
          writeInt16(out, newIndex);
          out.write(type.getTag());
          writeString(out, str);
        } else {
          out.write(kGetReference);
          writeInt16(out, index);
        }
      }
      break;
    }
    case BLOB: {
      out.write(type.getTag());
      PBlob blob = (PBlob) value;
      int length = blob.length();
      writeInt32(out, length);
      for (int i = 0; i < length; i++)
        out.write(blob.get(i));
      break;
    }
    case MAP:
      out.write(type.getTag());
      PMap map = (PMap) value;
      writeInt32(out, map.size());
      map.forEach(new ThrowingThunk<IOException>() {
        @Override
        public boolean call(PValue key, PValue value) throws IOException {
          write(out, key, data);
          write(out, value, data);
          return true;
        }
      });
      break;
    case INTEGER:
      out.write(type.getTag());
      writeInt32(out, ((PInteger) value).getValue());
      break;
    case ARRAY: {
      out.write(type.getTag());
      PArray elms = (PArray) value;
      int length = elms.length();
      writeInt32(out, length);
      for (int i = 0; i < length; i++)
        write(out, elms.get(i), data);
      break;
    }
    case SEED:
      out.write(type.getTag());
      PSeed seed = (PSeed) value;
      write(out, seed.getTag(), data);
      write(out, seed.getPayload(), data);
      break;
    default:
      assert false;
    }
  }

  public PValue read(InputStream in) throws IOException {
    return read(in, new CodecData());
  }

  private PValue read(InputStream in, CodecData data) throws IOException {
    int tag = in.read();
    switch (tag) {
    case kNewReference: {
      int index = readInt16(in);
      PValue value = read(in, data);
      data.map(index, value);
      return value;
    }
    case kGetReference: {
      int index = readInt16(in);
      return (PValue) data.getObject(index);
    }
    case kStringTag:
      String str = readString(in);
      return Plankton.newString(str);
    case kMapTag: {
      int size = readInt32(in);
      Map<PValue, PValue> map = new HashMap<PValue, PValue>();
      for (int i = 0; i < size; i++) {
        PValue key = read(in, data);
        PValue value = read(in, data);
        map.put(key, value);
      }
      return Plankton.newMap(map);
    }
    case kBlobTag:
      int size = readInt32(in);
      byte[] bytes = new byte[size];
      in.read(bytes);
      return Plankton.newBlob(bytes);
    case kIntegerTag:
      return Plankton.newInteger(readInt32(in));
    case kArrayTag: {
      int length = readInt32(in);
      List<PValue> elms = new ArrayList<PValue>();
      for (int i = 0; i < length; i++)
        elms.add(read(in, data));
      return new ArrayImpl(elms);
    }
    case kSeedTag: {
      PValue seedTag = read(in, data);
      PValue payload = read(in, data);
      return new SeedImpl(seedTag, payload);
    }
    default:
      assert false;
      return null;
    }
  }

  private static void writeString(OutputStream out, String value) throws IOException {
    int length = value.length();
    writeInt32(out, length);
    for (int i = 0; i < length; i++)
      out.write(value.charAt(i) & 0xFF);
  }

  private static String readString(InputStream in) throws IOException {
    int length = readInt32(in);
    char[] chars = new char[length];
    for (int i = 0; i < length; i++)
      chars[i] = (char) in.read();
    return new String(chars);
  }

  private static void writeInt32(OutputStream out, int value) throws IOException {
    out.write((value >> 0) & 0xFF);
    out.write((value >> 8) & 0xFF);
    out.write((value >> 16) & 0xFF);
    out.write((value >> 24) & 0xFF);
  }

  private static void writeInt16(OutputStream out, int value) throws IOException {
    out.write((value >> 0) & 0xFF);
    out.write((value >> 8) & 0xFF);
  }

  private static int readInt32(InputStream in) throws IOException {
    return (in.read() << 0)
         | (in.read() << 8)
         | (in.read() << 16)
         | (in.read() << 24);
  }

  private static int readInt16(InputStream in) throws IOException {
    return (in.read() << 0)
         | (in.read() << 8);
  }

}
