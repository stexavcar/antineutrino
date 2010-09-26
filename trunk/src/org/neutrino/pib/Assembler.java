package org.neutrino.pib;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;

import org.neutrino.runtime.Native;
import org.neutrino.runtime.RValue;


public class Assembler {

  private final ByteArrayOutputStream bytes = new ByteArrayOutputStream();
  private final List<Object> literals = new ArrayList<Object>();

  public void literal(int value) {
    add(Opcode.kNumber);
    add(value);
  }

  public void push(RValue value) {
    int index = registerLiteral(value);
    add(Opcode.kPush);
    add(index);
  }

  public void nuhll() {
    add(Opcode.kNull);
  }

  public void thrue() {
    add(Opcode.kTrue);
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

  public void callNative(Native method) {
    int index = registerLiteral(method);
    add(Opcode.kCallNative);
    add(index);
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
