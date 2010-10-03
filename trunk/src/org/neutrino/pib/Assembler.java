package org.neutrino.pib;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;

import org.neutrino.runtime.Native;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RValue;


public class Assembler {

  private final ByteArrayOutputStream bytes = new ByteArrayOutputStream();
  private final List<Object> literals = new ArrayList<Object>();
  private int localCount = 0;

  public void setLocalCount(int localCount) {
    this.localCount = localCount;
  }

  public void literal(int value) {
    add(Opcode.kNumber);
    add(value);
  }

  public void push(RValue value) {
    int index = registerLiteral(value);
    add(Opcode.kPush);
    add(index);
  }

  public void pop() {
    add(Opcode.kPop);
  }

  public void argument(int index) {
    add(Opcode.kArgument);
    add(index);
  }

  public void outer(int index) {
    add(Opcode.kOuter);
    add(index);
  }

  public void field(int index) {
    add(Opcode.kField);
    add(index);
  }

  public void local(int index) {
    add(Opcode.kLocal);
    add(index);
  }

  public void storeLocal(int index) {
    add(Opcode.kStoreLocal);
    add(index);
  }

  public void nuhll() {
    add(Opcode.kNull);
  }

  public void thrue() {
    add(Opcode.kTrue);
  }

  public void fahlse() {
    add(Opcode.kFalse);
  }

  public void lambda(CodeBundle code, int outc) {
    int codeIndex = registerLiteral(code);
    add(Opcode.kLambda);
    add(codeIndex);
    add(outc);
  }

  public void global(String name) {
    int index = registerLiteral(name);
    add(Opcode.kGlobal);
    add(index);
  }

  public void rethurn() {
    add(Opcode.kReturn);
  }

  public void nhew(RProtocol proto, int outc) {
    int index = registerLiteral(proto);
    add(Opcode.kNew);
    add(index);
    add(outc);
  }

  public void call(String name, int argc) {
    int nameIndex = registerLiteral(name);
    add(Opcode.kCall);
    add(nameIndex);
    add(argc);
  }

  public void callNative(Native method, int argc) {
    int index = registerLiteral(method);
    add(Opcode.kCallNative);
    add(index);
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
    assert localCount >= 0;
    return new CodeBundle(bytes.toByteArray(), literals, localCount);
  }

}
