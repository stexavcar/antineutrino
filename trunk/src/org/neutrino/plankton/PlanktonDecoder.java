package org.neutrino.plankton;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class PlanktonDecoder {

  private final LowLevelDecoder in;

  public PlanktonDecoder(InputStream in) {
    this.in = new LowLevelDecoder(in);
  }

  public Object read() throws IOException {
    int tag = in.read();
    switch (tag) {
    case PlanktonEncoder.kIntTag:
      return (int) in.readSigned();
    case PlanktonEncoder.kStringTag:
      return parseString();
    case PlanktonEncoder.kMapTag:
      return parseMap();
    case PlanktonEncoder.kArrayTag:
      return parseArray();
    default:
      throw new AssertionError("Unknown tag: " + tag);
    }
  }

  private String parseString() throws IOException {
    int length = (int) in.readUnsigned();
    byte[] bytes = new byte[length];
    for (int i = 0; i < length; i++)
      bytes[i] = (byte) in.read();
    return new String(bytes);
  }

  private List<?> parseArray() throws IOException {
    int length = (int) in.readUnsigned();
    ArrayList<Object> result = new ArrayList<Object>(length);
    for (int i = 0; i < length; i++)
      result.add(read());
    return result;
  }

  private Map<?, ?> parseMap() throws IOException {
    int length = (int) in.readUnsigned();
    HashMap<Object, Object> result = new HashMap<Object, Object>();
    for (int i = 0; i < length; i++) {
      Object key = read();
      Object value = read();
      result.put(key, value);
    }
    return result;
  }

}
