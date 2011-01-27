package org.javatrino.bytecode;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Expression.AddIntrinsics;
import org.javatrino.ast.Expression.Block;
import org.javatrino.ast.Expression.Call;
import org.javatrino.ast.Expression.Call.Argument;
import org.javatrino.ast.Expression.Constant;
import org.javatrino.ast.Expression.Definition;
import org.javatrino.ast.Expression.GetField;
import org.javatrino.ast.Expression.Global;
import org.javatrino.ast.Expression.Local;
import org.javatrino.ast.Expression.NewObject;
import org.javatrino.ast.Expression.SetField;
import org.javatrino.ast.Expression.TagWithProtocol;
import org.javatrino.ast.Expression.Visitor;
import org.javatrino.ast.Method;
import org.javatrino.ast.Symbol;
import org.neutrino.pib.Assembler;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RString;

public class BytecodeCompiler extends Visitor {

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
            rewrite.getValue().accept(BytecodeCompiler.this);
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
    expr.accept(compiler);
    assm.rethurn();
    return compiler.getResult();
  }

  private static List<Symbol> getLocals(Expression expr) {
    final List<Symbol> result = new ArrayList<Symbol>();
    expr.accept(new Visitor() {
      @Override
      public void visitDefinition(Definition that) {
        result.add(that.symbol);
        super.visitDefinition(that);
      }
    });
    return result;
  }

  class CapturingRewriter extends Visitor {

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
    public void visitLocal(Local that) {
      Symbol sym = that.symbol;
      if (shouldCapture(sym)) {
        Symbol self = method.signature.get(1).symbol;
        rewrites.put(sym, new GetField(new Local(self), getFieldKey(sym)));
      }
    }

    @Override
    public void visitAddIntrinsics(AddIntrinsics that) {
      for (Method method : that.methods) {
        method.body.accept(this);
      }
    }

    public Map<Symbol, Expression> getRewrites(Expression expr) {
      expr.accept(this);
      return this.rewrites;
    }

  }

  @Override
  public void visitGlobal(Global that) {
    assm.global(that.name);
  }

  @Override
  public void visitConstant(Constant that) {
    Object value = that.value;
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

  @Override
  public void visitBlock(Block that) {
    List<Expression> values = that.values;
    assert values.size() > 1;
    boolean isFirst = true;
    for (Expression value : values) {
      if (isFirst) {
        isFirst = false;
      } else {
        assm.pop();
      }
      value.accept(this);
    }
  }

  public void visitLocal(Local that) {
    loaders.get(that.symbol).load(assm);
  }

  @Override
  public void visitDefinition(Definition that) {
    that.value.accept(this);
    loaders.get(that.symbol).store(assm);
    that.body.accept(this);
  }

  @Override
  public void visitCall(Call that) {
    List<Argument> args = that.arguments;
    Argument nameArg = args.get(0);
    assert nameArg.tag.equals("name");
    String name = (String) ((Constant) nameArg.value).value;
    for (int i = 1; i < args.size(); i++) {
      Argument arg = args.get(i);
      assert arg.tag.equals(i - 1);
      arg.value.accept(this);
    }
    assm.call(name, Collections.nCopies(args.size() - 1, 0));
  }

  @Override
  public void visitAddIntrinsics(AddIntrinsics that) {
    that.object.accept(this);
    Map<Symbol, RFieldKey> captured = new HashMap<Symbol, RFieldKey>();
    List<Method> rewrittenMethods = new ArrayList<Method>();
    for (final Method method : that.methods) {
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

  @Override
  public void visitSetField(SetField that) {
    that.object.accept(this);
    that.value.accept(this);
    assm.setField(that.field);
  }

  @Override
  public void visitGetField(GetField that) {
    that.object.accept(this);
    assm.getField(that.field);
  }

  @Override
  public void visitTagWithProtocol(TagWithProtocol that) {
    that.object.accept(this);
    for (Expression proto : that.protocols)
      proto.accept(this);
    assm.tagWithProtocols(that.protocols.size());
  }

  @Override
  public void visitNewObject(NewObject that) {
    assm.newObject();
  }

  public Result getResult() {
    return new Result(assm.getBytecode(), assm.getLiterals(), localCount);
  }

}
