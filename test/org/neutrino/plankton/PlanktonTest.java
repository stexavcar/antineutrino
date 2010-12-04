package org.neutrino.plankton;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

public class PlanktonTest extends TestCase {

  private class PlanktonHelper {

    private final ByteArrayOutputStream bytes = new ByteArrayOutputStream();
    private final PlanktonEncoder out = new PlanktonEncoder(bytes);

    public Object readBack() throws IOException {
      InputStream in = new ByteArrayInputStream(bytes.toByteArray());
      return new PlanktonDecoder(in).read();
    }

  }

  private void runTest(Object value) throws IOException {
    PlanktonHelper helper = new PlanktonHelper();
    helper.out.write(value);
    byte[] bytes = helper.bytes.toByteArray();
    System.out.println(value + ": " + bytes.length);
    Object result = helper.readBack();
    assertEquals(value, result);
  }

  public void testSimple() throws IOException {
    runTest(Arrays.asList(1, 2, 3, 4, 5));
    runTest(1);
    runTest("Hello World");
    Map<Object, Object> map = new HashMap<Object, Object>();
    map.put("foo", "bar");
    map.put(2, Arrays.asList("a", "b", "c"));
    runTest(map);
  }

}
