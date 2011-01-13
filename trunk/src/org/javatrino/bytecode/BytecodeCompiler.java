package org.javatrino.bytecode;

import java.util.List;

import org.javatrino.ast.Expression;
import org.neutrino.pib.Assembler;

public class BytecodeCompiler {

  public class Result {
    public final byte[] code;
    public final List<Object> literals;
    public Result(byte[] code, List<Object> literals) {
      this.code = code;
      this.literals = literals;
    }
  }

  private final Assembler assm;

  public BytecodeCompiler(Assembler assm) {
    this.assm = assm;
  }

  public static Result compile(Expression expr) {
    Assembler assm = new Assembler(null);
    BytecodeCompiler compiler = new BytecodeCompiler(assm);
    compiler.emit(expr);
    assm.rethurn();
    return compiler.getResult();
  }

  public void emit(Expression expr) {
    switch (expr.kind) {
    case GLOBAL: {
      Expression.Global global = (Expression.Global) expr;
      assm.global(global.name);
      break;
    }
    default:
      throw new AssertionError(expr.kind);
    }
  }

  public Result getResult() {
    return new Result(assm.getBytecode(), assm.getLiterals());
  }

}
