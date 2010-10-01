package org.neutrino.runtime;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;

import com.sun.xml.internal.messaging.saaj.util.ByteOutputStream;

public class RFile extends RValue {

  private static final TypeId TYPE_ID = TypeId.get("file");

  private final File file;

  public RFile(File file) {
    this.file = file;
  }

  @Override
  public TypeId getTypeId() {
    return TYPE_ID;
  }

  @Override
  public String toString() {
    return "#<a File: " + file + ">";
  }

  public byte[] read() {
    try {
      FileInputStream in = new FileInputStream(file);
      byte[] buffer = new byte[256];
      ByteOutputStream out = new ByteOutputStream();
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

}
