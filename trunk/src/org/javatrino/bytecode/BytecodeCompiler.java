package org.javatrino.bytecode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Expression.AddIntrinsics;
import org.javatrino.ast.Expression.Call;
import org.javatrino.ast.Expression.Call.Argument;
import org.javatrino.ast.Expression.Constant;
import org.javatrino.ast.Expression.Definition;
import org.javatrino.ast.Expression.GetField;
import org.javatrino.ast.Expression.Local;
import org.javatrino.ast.Expression.Visitor;
import org.javatrino.ast.Method;
import org.javatrino.ast.Symbol;
import org.neutrino.pib.Assembler;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RString;

public class BytecodeCompiler {

  public class Result {
    public final byte[] code;
    public final List<Object> literals;
    public final int localCount;
    public Result(byte[] code, List<Object> literals, int localCount) {
      this.code = code;
      this.literals = literals;
      this.localCount = localCount;
    }
  }

  private abstract class LocalAccess {

    public abstract void load(Assembler assm);

    public void store(Assembler assm) {
      assert false;
    }

  }

  private final Map<Symbol, LocalAccess> loaders = new HashMap<Symbol, LocalAccess>();
  private final Assembler assm;
  private int localCount = -1;

  public BytecodeCompiler(Assembler assm, final List<Symbol> params,
      final List<Symbol> locals, final Map<Symbol, Expression> rewrites) {
    this.assm = assm;
    for (int i = 0; i < params.size(); i++) {
      final int index = i;
      loaders.put(params.get(i), new LocalAccess() {
        @Override
        public void load(Assembler assm) {
          assm.argument(params.size() - index);
        }
      });
    }
    this.localCount = locals.size();
    for (int i = 0; i < locals.size(); i++) {
      final int index = i;
      loaders.put(locals.get(i), new LocalAccess() {
        @Override
        public void load(Assembler assm) {
          assm.local(index);
        }
        @Override
        public void store(Assembler assm) {
          assm.storeLocal(index);
        }
      });
    }
    if (rewrites != null) {
      for (final Map.Entry<Symbol, Expression> rewrite : rewrites.entrySet()) {
        loaders.put(rewrite.getKey(), new LocalAccess() {
          @Override
          public void load(Assembler assm) {
            emit(rewrite.getValue());
          }
        });
      }
    }
  }

  public static int compilationCount = 0;

  public static Result compile(Expression expr, List<Symbol> params,
      Map<Symbol, Expression> rewrites) {
    compilationCount++;
    Assembler assm = new Assembler(null);
    BytecodeCompiler compiler = new BytecodeCompiler(assm, params, getLocals(expr),
        rewrites);
    compiler.emit(expr);
    assm.rethurn();
    return compiler.getResult();
  }

  private static List<Symbol> getLocals(Expression expr) {
    final List<Symbol> result = new ArrayList<Symbol>();
    expr.accept(new Visitor<Void>() {
      @Override
      public Void visitDefinition(Definition that) {
        result.add(that.symbol);
        return super.visitDefinition(that);
      }
    });
    return result;
  }

  private class CapturingRewriter extends Visitor<Void> {

    private final Map<Symbol, RFieldKey> captured;
    private final Map<Symbol, Expression> rewrites = new HashMap<Symbol, Expression>();
    private final Method method;

    public CapturingRewriter(Method method, Map<Symbol, RFieldKey> captured) {
      this.method = method;
      this.captured = captured;
    }

    private RFieldKey getFieldKey(Symbol symbol) {
      if (!captured.containsKey(symbol)) {
        captured.put(symbol, new RFieldKey(symbol));
      }
      return captured.get(symbol);
    }

    private boolean inScope(Symbol sym) {
      return !loaders.containsKey(sym);
    }

    private boolean hasBeenCaptured(Symbol sym) {
      return rewrites.containsKey(sym);
    }

    private boolean shouldCapture(Symbol sym) {
      return !inScope(sym) && !hasBeenCaptured(sym);
    }

    @Override
    public Void visitLocal(Local that) {
      Symbol sym = that.symbol;
      if (shouldCapture(sym)) {
        Symbol self = method.signature.get(1).symbol;
        rewrites.put(sym, new GetField(new Local(self), getFieldKey(sym)));
      }
      return null;
    }

    @Override
    public Void visitAddIntrinsics(AddIntrinsics that) {
      for (Method method : that.methods) {
        method.body.accept(this);
      }
      return null;
    }

    public Map<Symbol, Expression> getRewrites(Expression expr) {
      expr.accept(this);
      return this.rewrites;
    }

  }

  private void emitAddIntrinsics(AddIntrinsics add) {
    emit(add.object);
    Map<Symbol, RFieldKey> captured = new HashMap<Symbol, RFieldKey>();
    List<Method> rewrittenMethods = new ArrayList<Method>();
    for (final Method method : add.methods) {
      Map<Symbol, Expression> rewrites =
          new CapturingRewriter(method, captured).getRewrites(method.body);
      rewrittenMethods.add(method.withRewrites(rewrites));
    }
    for (Map.Entry<Symbol, RFieldKey> entry : captured.entrySet()) {
      assm.dup();
      loaders.get(entry.getKey()).load(assm);
      assm.setField(entry.getValue());
      assm.pop();
    }
    assm.addIntrinsics(rewrittenMethods);
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
    case LOCAL: {
      Expression.Local local = (Expression.Local) expr;
      loaders.get(local.symbol).load(assm);
      break;
    }
    case DEF: {
      Expression.Definition def = (Expression.Definition) expr;
      emitLocalDefinition(def);
      break;
    }
    case CALL: {
      Expression.Call call = (Expression.Call) expr;
      emitCall(call);
      break;
    }
    case ADD_INTRINSICS: {
      Expression.AddIntrinsics add = (Expression.AddIntrinsics) expr;
      emitAddIntrinsics(add);
      break;
    }
    case SET_FIELD: {
      Expression.SetField set = (Expression.SetField) expr;
      emit(set.object);
      emit(set.value);
      assm.setField(set.field);
      break;
    }
    case GET_FIELD: {
      Expression.GetField get = (Expression.GetField) expr;
      emit(get.object);
      assm.getField(get.field);
      break;
    }
    case TAG_WITH_PROTOCOL: {
      Expression.TagWithProtocol tag = (Expression.TagWithProtocol) expr;
      emit(tag.object);
      for (Expression proto : tag.protocols)
        emit(proto);
      assm.tagWithProtocols(tag.protocols.size());
      break;
    }
    case NEW: {
      assm.newObject();
      break;
    }
    default:
      throw new AssertionError(expr.kind);
    }
  }
  private void emitCall(Call call) {
    List<Argument> args = call.arguments;
    Argument nameArg = args.get(0);
    assert nameArg.tag.equals("name");
    String name = (String) ((Constant) nameArg.value).value;
    for (int i = 1; i < args.size(); i++) {
      Argument arg = args.get(i);
      assert arg.tag.equals(i - 1);
      emit(arg.value);
    }
    assm.call(name, Collections.nCopies(args.size() - 1, 0));
  }

  private void emitLocalDefinition(Definition def) {
    emit(def.value);
    loaders.get(def.symbol).store(assm);
    emit(def.body);
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
    return new Result(assm.getBytecode(), assm.getLiterals(), localCount);
  }

}
