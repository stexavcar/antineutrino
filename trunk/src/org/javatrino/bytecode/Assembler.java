package org.javatrino.bytecode;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;

import org.javatrino.ast.Method;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RValue;


public class Assembler {

  private final ByteArrayOutputStream bytes = new ByteArrayOutputStream();
  private final List<Object> literals = new ArrayList<Object>();

  private int getOffset() {
    return bytes.size();
  }

  public int push(RValue value) {
    int index = registerLiteral(value);
    int result = getOffset();
    add(Opcode.kLiteral);
    add(index);
    return result;
  }

  public void pop() {
    add(Opcode.kPop);
  }

  public void dup() {
    add(Opcode.kDup);
  }

  public void argument(int index) {
    add(Opcode.kArgument);
    add(index);
  }

  public int outer(RFieldKey field) {
    int index = registerLiteral(field);
    int result = getOffset();
    add(Opcode.kOuter);
    add(index);
    return result;
  }

  public void setField(RFieldKey field) {
    add(Opcode.kSetField);
    add(registerLiteral(field));
  }

  public void getField(RFieldKey field) {
    add(Opcode.kGetField);
    add(registerLiteral(field));
  }

  public void tagWithProtocols(int count) {
    add(Opcode.kTagWithProtocols);
    add(count);
  }

  public void newObject() {
    add(Opcode.kNewObject);
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

  public int defineLocal(int index, int value, int body) {
    int result = getOffset();
    add(Opcode.kDefineLocal);
    add(index);
    add(value);
    add(body);
    return result;
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

  public int global(Object name) {
    int index = registerLiteral(name);
    int result = getOffset();
    add(Opcode.kGlobal);
    add(index);
    return result;
  }

  public void addIntrinsics(List<Method> methods) {
    int index = registerLiteral(methods);
    add(Opcode.kAddIntrinsics);
    add(index);
  }


  public void rethurn() {
    add(Opcode.kReturn);
  }

  public int nhew(RProtocol proto, List<RFieldKey> fields) {
	int result = getOffset();
	int protoIndex = registerLiteral(proto);
	int fieldsIndex = registerLiteral(fields);
    add(Opcode.kNew);
    add(protoIndex);
    add(fieldsIndex);
    return result;
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
    add(0xFF);
    for (int arg : args)
      add(arg);
    return result;
  }

  public int block(List<Integer> elm) {
    int result = getOffset();
    add(Opcode.kBlock);
    add(elm.size());
    for (int arg : elm)
      add(arg);
    return result;
  }

  public int with1cc() {
    int result = getOffset();
    add(Opcode.kWith1cc);
    add(0);
    add(2);
    add(0);
    add(0);
    add(0);
    return result;
  }

  public int withEscape() {
    int result = getOffset();
    add(Opcode.kWithEscape);
    add(0);
    add(2);
    add(0);
    add(0);
    add(0);
    return result;
  }


  public void callNative(Native method, int argc) {
    int index = registerLiteral(method);
    add(Opcode.kCallNative);
    add(index);
    add(argc);
    add(0);
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

  public byte[] getBytecode() {
    return bytes.toByteArray();
  }

  public List<Object> getLiterals() {
    return literals;
  }

}