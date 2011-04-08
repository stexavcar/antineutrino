package org.neutrino.plankton;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.neutrino.pib.Universe;

public class PlanktonTest extends TestCase {

  private class PlanktonHelper {

    private final PlanktonEncoder out;
    private final ClassIndex index;

    public PlanktonHelper(ClassIndex index) {
      this.index = index;
      this.out = new PlanktonEncoder(index, Universe.getBuiltinIndex());
    }

    public PlanktonHelper() {
      this(new ClassIndex());
    }

    public Object readBack() throws IOException {
      InputStream in = new ByteArrayInputStream(out.getBytes());
      return new PlanktonDecoder(index, Universe.getBuiltinIndex(), in).read();
    }

    private Object encdec(Object value) throws IOException {
      out.write(value);
      return readBack();
    }

  }

  private Object encdec(Object value) throws IOException {
    return new PlanktonHelper().encdec(value);
  }

  private void runTest(Object value) throws IOException {
    Object result = encdec(value);
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

  public void testCircularArray() throws IOException {
    ArrayList<Object> objs = new ArrayList<Object>();
    objs.add(objs);
    List<?> result = (List<?>) encdec(objs);
    assertTrue(result.get(0) == result);
  }

  public void testRepeatedArray() throws IOException {
    ArrayList<Object> outer = new ArrayList<Object>();
    List<?> inner = Arrays.asList(3, 4, 5);
    outer.add(inner);
    outer.add(inner);
    outer.add(inner);
    List<?> result = (List<?>) encdec(outer);
    Object first = result.get(0);
    assertEquals(inner, first);
    assertTrue(result.get(1) == first);
    assertTrue(result.get(2) == first);
  }

  private static class Point {

    public @Store int x;
    public @Store int y;

    public Point(int x, int y) {
      this.x = x;
      this.y = y;
    }

    @SuppressWarnings("unused")
    public Point() { }

  }

  private static class Rect {

    public @Store Point top_left;
    public @Store Point bottom_right;

    public Rect(Point top_left, Point bottom_right) {
      this.top_left = top_left;
      this.bottom_right = bottom_right;
    }

    @SuppressWarnings("unused")
    public Rect() { }

  }

  public void testClasses() throws IOException {
    PlanktonHelper helper = new PlanktonHelper(new ClassIndex() {{
      add(Point.class);
    }});
    Point p = (Point) helper.encdec(new Point(2, 3));
    assertEquals(2, p.x);
    assertEquals(3, p.y);
  }

  public void testClassPointers() throws IOException {
    PlanktonHelper helper = new PlanktonHelper(new ClassIndex() {{
      add(Point.class);
      add(Rect.class);
    }});
    Point p = new Point(4, 5);
    Rect r = (Rect) helper.encdec(new Rect(p, p));
    assertTrue(r.top_left == r.bottom_right);
    assertEquals(4, r.top_left.x);
    assertEquals(5, r.top_left.y);
  }

}
