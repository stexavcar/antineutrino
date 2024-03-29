package org.javatrino.bytecode;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;

import org.javatrino.ast.Method;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RValue;
import org.neutrino.runtime.lookup.CallInfo;


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

  public int global(RValue name) {
    int index = registerLiteral(name);
    int result = getOffset();
    add(Opcode.kGlobal);
    add(index);
    return result;
  }

  public void addIntrinsics(List<Method> methods) {
    if (methods.size() == 2 && methods.get(0).signature.toString().equals(methods.get(1).signature.toString())) {
      System.out.println("!!!");
    }
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

  public void newObjectArray(int elmc) {
    add(Opcode.kNewObjectArray);
    add(elmc);
  }

  public void newDictionary(int elmc) {
    add(Opcode.kNewDictionary);
    add(elmc);
  }

  public int call(CallInfo info) {
    int infoIndex = registerLiteral(info);
    int result = getOffset();
    add(Opcode.kCall);
    add(infoIndex);
    add(info.getArgumentCount());
    add(0xFF);
    return result;
  }

  public int withEscape() {
    int result = getOffset();
    add(Opcode.kWithEscape);
    add(0);
    add(3);
    add(0);
    return result;
  }


  public void callNative(Native method, int argc) {
    int index = registerLiteral(method);
    add(Opcode.kCallNative);
    add(index);
    add(argc);
    add(0);
  }

  public int registerLiteral(Object obj) {
    int index = literals.indexOf(obj);
    if (index == -1) {
      index = literals.size();
      literals.add(obj);
    }
    assert index < 256;
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
