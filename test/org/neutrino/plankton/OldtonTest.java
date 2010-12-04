package org.neutrino.plankton;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;

import junit.framework.TestCase;

import org.neutrino.oldton.ISeedable;
import org.neutrino.oldton.PArray;
import org.neutrino.oldton.PInteger;
import org.neutrino.oldton.PMap;
import org.neutrino.oldton.PSeed;
import org.neutrino.oldton.PString;
import org.neutrino.oldton.PValue;
import org.neutrino.oldton.Oldton;
import org.neutrino.oldton.OldtonRegistry;
import org.neutrino.oldton.PValue.Type;
import org.neutrino.oldton.annotations.Growable;
import org.neutrino.oldton.annotations.SeedMember;

/**
 * Test of the {@link Oldton} utility class.
 *
 * @author plesner@google.com (Christian Plesner Hansen)
 */
public class OldtonTest extends TestCase {

  private OldtonRegistry registry;
  private Oldton plankton;

  @Override
  protected void setUp() throws Exception {
    registry = new OldtonRegistry();
    plankton = new Oldton(registry);
  }

  public void testTypes() {
    assertEquals(Type.STRING, Oldton.newString("foo").getType());
    assertEquals(Type.BLOB, Oldton.newBlob(new byte[0]).getType());
    assertEquals(Type.MAP, Oldton.newMap(Collections.<PValue, PValue>emptyMap()).getType());
  }

  public void testStringEncoding() {
    byte[] data = marshal(Oldton.newString("hey!"));
    PValue result = unmarshal(data);
    assertEquals(Type.STRING, result.getType());
    assertEquals("hey!", ((PString) result).getValue());
  }

  public void testLongEncoding() {
    int value = (int) 4277009102L;
    byte[] data = marshal(Oldton.newInteger(value));
    PValue result = unmarshal(data);
    assertEquals(value, ((PInteger) result).getValue());
  }

  public void testCompositeEncoding() {
    final PMap original = Oldton.newMap(new HashMap<PString, PValue>() {{
      put(Oldton.newString("a"), Oldton.newInteger(4));
      put(Oldton.newString("b"), Oldton.newInteger(7));
      put(Oldton.newString("c"), Oldton.newString("8"));
    }});
    byte[] data = marshal(original);
    PValue value = unmarshal(data);
    assertEquals(Type.MAP, value.getType());
    PMap map = (PMap) value;
    map.forEach(new PMap.Thunk() {
      public boolean call(PValue key, PValue value) {
        assertEquals(Type.STRING, key.getType());
        assertEquals(original.get(key, null), value);
        return true;
      }
    });
  }

  public void testArrayEncoding() {
    PArray array = Oldton.newArray(Arrays.asList(
      Oldton.newString("a"),
      Oldton.newInteger(4),
      Oldton.newArray(Arrays.asList(
        Oldton.newString("b")
      )),
      Oldton.newString("c")
    ));
    byte[] data = marshal(array);
    PValue value = unmarshal(data);
    assertEquals(Type.ARRAY, value.getType());
    PArray result = (PArray) value;
    assertEquals(4, result.length());
    assertEquals(array.get(0), result.get(0));
    assertEquals(array.get(1), result.get(1));
    PValue thirdValue = result.get(2);
    assertEquals(Type.ARRAY, thirdValue.getType());
    PArray thirdArray = (PArray) thirdValue;
    assertEquals(1, thirdArray.length());
    assertEquals(Oldton.newString("b"), thirdArray.get(0));
    assertEquals(array.get(3), result.get(3));
  }

  @Growable(Point.TAG)
  private static class Point implements ISeedable {

    public static final String TAG = "org::neutrino::plankton::PlanktonTest::Point";

    @SeedMember public int x;
    @SeedMember public int y;

    public Point(int x, int y) {
      this.x = x;
      this.y = y;
    }

    public Point() { }

    public int getX() {
      return this.x;
    }

    public int getY() {
      return this.y;
    }

  }

  public void testSeeds() {
    registry.register(Point.class);
    Point p = new Point(2, 3);
    PSeed seed = plankton.newSeed(p);
    byte[] encoded = marshal(seed);
    PValue value = unmarshal(encoded);
    assertEquals(Type.SEED, value.getType());
    Point result = ((PSeed) value).grow(Point.class);
    assertEquals(2, result.getX());
    assertEquals(3, result.getY());
  }

  public void testRepetition() {
    PString str = Oldton.newString("foobarbaz");
    PValue value = Oldton.newArray(str, str, str);
    byte[] encoded = marshal(value);
    unmarshal(encoded);
  }

  private final byte[] marshal(PValue value) {
    try {
      ByteArrayOutputStream out = new ByteArrayOutputStream();
      plankton.write(out, value);
      out.flush();
      return out.toByteArray();
    } catch (IOException ioe) {
      fail(ioe.toString());
      return null;
    }
  }

  private final PValue unmarshal(byte[] data) {
    try {
      ByteArrayInputStream in = new ByteArrayInputStream(data);
      return plankton.read(in);
    } catch (IOException ioe) {
      fail(ioe.toString());
      return null;
    }
  }

}
