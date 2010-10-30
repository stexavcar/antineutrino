package org.neutrino.pib;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;

import org.neutrino.compiler.Source;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RValue;


public class Assembler {

  private final Source origin;
  private final ByteArrayOutputStream bytes = new ByteArrayOutputStream();
  private final List<Object> literals = new ArrayList<Object>();
  private int localCount = -1;
  private int rootOffset = -1;

  public Assembler(Source origin) {
    this.origin = origin;
  }

  public void finalize(int localCount, int rootOffset) {
    assert this.localCount == -1;
    assert this.rootOffset == -1;
    this.localCount = localCount;
    this.rootOffset = rootOffset;
  }

  private int getOffset() {
    return bytes.size();
  }

  public int push(RValue value) {
    int index = registerLiteral(value);
    int result = getOffset();
    add(Opcode.kPush);
    add(index);
    return result;
  }

  public void pop() {
    add(Opcode.kPop);
  }

  public int argument(int index) {
    int result = getOffset();
    add(Opcode.kArgument);
    add(index);
    return result;
  }

  public int outer(int index) {
    int result = getOffset();
    add(Opcode.kOuter);
    add(index);
    return result;
  }

  public void field(int index) {
    add(Opcode.kField);
    add(index);
  }

  public int local(int index) {
    int result = getOffset();
    add(Opcode.kLocal);
    add(index);
    return result;
  }

  public void storeLocal(int index) {
    add(Opcode.kStoreLocal);
    add(index);
  }

  public int nuhll() {
    int offset = getOffset();
    add(Opcode.kNull);
    return offset;
  }

  public int thrue() {
    int offset = getOffset();
    add(Opcode.kTrue);
    return offset;
  }

  public int fahlse() {
    int offset = getOffset();
    add(Opcode.kFalse);
    return offset;
  }

  public int global(String name) {
    int index = registerLiteral(name);
    int result = getOffset();
    add(Opcode.kGlobal);
    add(index);
    return result;
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

  public void newArray(int elmc) {
    add(Opcode.kNewArray);
    add(elmc);
  }

  public int call(String name, List<Integer> args) {
    int nameIndex = registerLiteral(name);
    int result = getOffset();
    add(Opcode.kCall);
    add(nameIndex);
    add(args.size());
    for (int arg : args)
      add(arg);
    return result;
  }

  public int with1Cc() {
    int result = getOffset();
    add(Opcode.kWith1Cc);
    add(0);
    add(2);
    add(0);
    add(0);
    return result;
  }

  public void callNative(Native method, int argc) {
    int index = registerLiteral(method);
    add(Opcode.kCallNative);
    add(index);
    add(argc);
    for (int i = 0; i < argc; i++)
      add(0);
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
    assert rootOffset != -1;
    return new CodeBundle(bytes.toByteArray(), literals, localCount,
        origin.getName(), rootOffset);
  }

}
