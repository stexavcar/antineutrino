package org.neutrino.pib;

import java.io.ByteArrayOutputStream;
import java.util.List;


public class Assembler {

  private final List<String> annots;
  private final ByteArrayOutputStream bytes = new ByteArrayOutputStream();

  public Assembler(List<String> annots) {
    this.annots = annots;
  }

  public void literal(int value) {
    add(Opcode.NUMBER);
    add(value);
  }

  public void lambda() {
    add(Opcode.LAMBDA);
  }

  private void add(Opcode opcode) {
    bytes.write(opcode.getCode());
  }

  private void add(int value) {
    bytes.write(value);
  }

  public Binding getCode() {
    return new Binding(annots, bytes.toByteArray());
  }

}
