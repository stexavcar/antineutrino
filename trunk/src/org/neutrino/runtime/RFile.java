package org.neutrino.runtime;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;

public class RFile extends RValue {

  private static final RProtocol[] PROTOS = RProtocol.getCanonicals("file");

  private final File file;

  public RFile(File file) {
    this.file = file;
  }

  @Override
  public RProtocol[] getTypeIds() {
    return PROTOS;
  }

  @Override
  public String toString() {
    return "#<a File: " + file + ">";
  }

  public byte[] read() {
    try {
      FileInputStream in = new FileInputStream(file);
      byte[] buffer = new byte[256];
      ByteArrayOutputStream out = new ByteArrayOutputStream();
      while (true) {
        int count = in.read(buffer);
        if (count <= 0) {
          break;
        } else {
          out.write(buffer, 0, count);
        }
      }
      return out.toByteArray();
    } catch (IOException ioe) {
      throw new RuntimeException(ioe);
    }
  }

  public void write(byte[] bytes) {
    try {
      FileOutputStream out = new FileOutputStream(file);
      out.write(bytes);
      out.close();
    } catch (IOException ioe) {
      throw new RuntimeException(ioe);
    }
  }

  @Override
  public State getState() {
    return State.IMMUTABLE;
  }

}
