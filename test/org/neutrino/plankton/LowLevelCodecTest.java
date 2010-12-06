package org.neutrino.plankton;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import junit.framework.TestCase;

public class LowLevelCodecTest extends TestCase {

  private static int toInt(byte b) {
    return ((int) b) & 0xFF;
  }

  private static class TestEncoder {

    private final LowLevelEncoder encoder;

    public TestEncoder() {
      encoder = new LowLevelEncoder();
    }

    public void writeBytes(int... values) {
      try {
        for (int value : values)
          encoder.write(value & 0xFF);
      } catch (IOException ioe) {
        throw new RuntimeException(ioe);
      }
    }

    public void flush() {
      try {
        encoder.flush();
      } catch (IOException ioe) {
        throw new RuntimeException(ioe);
      }
    }

    public void checkRawBytes(int... values) {
      this.flush();
      byte[] data = encoder.getBytes();
      List<Integer> found = new ArrayList<Integer>();
      List<Integer> expected = new ArrayList<Integer>();
      for (byte b : data)
        found.add(toInt(b));
      for (int value : values)
        expected.add(value);
      assertEquals(expected, found);
    }

    public void checkReadBytes(int... values) {
      this.flush();
      byte[] data = encoder.getBytes();
      LowLevelDecoder decoder = new LowLevelDecoder(new ByteArrayInputStream(data));
      List<Integer> expected = new ArrayList<Integer>();
      List<Integer> found = new ArrayList<Integer>();
      for (int i = 0; i < values.length; i++) {
        expected.add(values[i]);
        try {
          found.add(decoder.read());
        } catch (IOException ioe) {
          throw new RuntimeException(ioe);
        }
      }
      assertEquals(expected, found);
    }

    public LowLevelDecoder getDecoder() {
      this.flush();
      return new LowLevelDecoder(new ByteArrayInputStream(encoder.getBytes()));
    }

  }

  public void testSimpleEncodings() {
    TestEncoder empty = new TestEncoder();
    empty.writeBytes(0);
    empty.checkRawBytes(0, 0, 0, 0, 0);
    empty.checkReadBytes(0);
    TestEncoder full = new TestEncoder();
    full.writeBytes(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
    full.checkRawBytes(0xFF, 0xEF, 0xEF, 0xEF, 0xEF, 0xEF, 0xEF, 0xEF, 0xEF);
    full.checkReadBytes(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
    TestEncoder half = new TestEncoder();
    half.writeBytes(0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
    half.checkRawBytes(0x0F, 0, 0, 0xEF, 0xEF, 0xEF, 0xEF);
    half.checkReadBytes(0, 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF);
  }

  public void testRandomEncoding() {
    Random random = new Random(1119);
    int kValueCount = 256;
    int kIterationCount = 1024;
    // Test random data
    byte[] data = new byte[kValueCount];
    int[] values = new int[kValueCount];
    for (int i = 0; i < kIterationCount; i++) {
      TestEncoder encoder = new TestEncoder();
      random.nextBytes(data);
      for (int j = 0; j < kValueCount; j++)
        values[j] = toInt(data[j]);
      encoder.writeBytes(values);
      encoder.checkReadBytes(values);
    }
    // Test small values
    for (int i = 0; i < kIterationCount; i++) {
      TestEncoder encoder = new TestEncoder();
      random.nextBytes(data);
      for (int j = 0; j < kValueCount; j++) {
        data[j] &= 0x0F;
        values[j] = toInt(data[j]);
      }
      encoder.writeBytes(values);
      encoder.flush();
      // Check that the number of bits in the output is 5 times the
      // number of bytes in the input.
      assertEquals(5 * kValueCount, 8 * encoder.encoder.length());
      encoder.checkReadBytes(values);
    }
  }

  public void testUnsignedLongEncoding() throws IOException {
    Random random = new Random(1129);
    List<Long> numbers = new ArrayList<Long>();
    TestEncoder encoder = new TestEncoder();
    for (int i = 0; i < 1024; i++) {
      long value = Math.abs(random.nextLong());
      numbers.add(value);
      encoder.encoder.writeUnsigned(value);
    }
    LowLevelDecoder decoder = encoder.getDecoder();
    for (int i = 0; i < 1024; i++) {
      assertEquals((long) numbers.get(i), decoder.readUnsigned());
    }
  }

  public void testSignedLongEncoding() throws IOException {
    Random random = new Random(1139);
    List<Long> numbers = new ArrayList<Long>();
    TestEncoder encoder = new TestEncoder();
    for (int i = 0; i < 1024; i++) {
      long value = random.nextLong();
      numbers.add(value);
      encoder.encoder.writeSigned(value);
    }
    LowLevelDecoder decoder = encoder.getDecoder();
    for (int i = 0; i < 1024; i++) {
      assertEquals((long) numbers.get(i), decoder.readSigned());
    }
  }

}
