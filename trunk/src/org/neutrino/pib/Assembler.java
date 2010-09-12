package org.neutrino.pib;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;


public class Assembler {

  private final ByteArrayOutputStream bytes = new ByteArrayOutputStream();
  private final List<Object> literals = new ArrayList<Object>();

  public void literal(int value) {
    add(Opcode.kNumber);
    add(value);
  }

  public void slap(int height) {
    add(Opcode.kSlap);
    add(height);
  }

  public void lambda(CodeBundle code) {
    int codeIndex = registerLiteral(code);
    add(Opcode.kLambda);
    add(codeIndex);
  }

  public void rethurn() {
    add(Opcode.kReturn);
  }

  public void call(String name, int argc) {
    int nameIndex = registerLiteral(name);
    add(Opcode.kCall);
    add(nameIndex);
    add(argc);
  }

  public int registerLiteral(Object obj) {
    int index = literals.indexOf(obj);
    if (index == -1) {
      index = literals.size();
      literals.add(obj);
    }
    return index;
  }

  private void add(int value) {
    bytes.write(value);
  }

  public CodeBundle getCode() {
    return new CodeBundle(bytes.toByteArray(), literals);
  }

}
