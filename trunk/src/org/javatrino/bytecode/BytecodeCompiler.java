package org.javatrino.bytecode;

import java.util.List;

import org.javatrino.ast.Expression;
import org.neutrino.pib.Assembler;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RString;

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
    case CONST: {
      Expression.Constant cons = (Expression.Constant) expr;
      emitConstant(cons.value);
      break;
    }
    case BLOCK: {
      Expression.Block block = (Expression.Block) expr;
      emitBlock(block.values);
      break;
    }
    default:
      throw new AssertionError(expr.kind);
    }
  }

  private void emitConstant(Object value) {
    if (value == null) {
      assm.nuhll();
    } else if (value == Boolean.TRUE) {
      assm.thrue();
    } else if (value == Boolean.FALSE) {
      assm.fahlse();
    } else if (value instanceof String) {
      assm.push(RString.of((String) value));
    } else if (value instanceof Integer) {
      assm.push(RInteger.get((Integer) value));
    } else {
      assert false : value.getClass();
    }
  }

  private void emitBlock(List<Expression> values) {
    assert values.size() > 1;
    boolean isFirst = true;
    for (Expression value : values) {
      if (isFirst) {
        isFirst = false;
      } else {
        assm.pop();
      }
      emit(value);
    }
  }

  public Result getResult() {
    return new Result(assm.getBytecode(), assm.getLiterals());
  }

}
