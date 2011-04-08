package org.javatrino.bytecode;

import static org.javatrino.ast.Expression.StaticFactory.eAddIntrinsics;
import static org.javatrino.ast.Expression.StaticFactory.eGetField;
import static org.javatrino.ast.Expression.StaticFactory.eLocal;
import static org.javatrino.ast.Expression.StaticFactory.eNewObject;

import java.util.ArrayList;
import java.util.Arrays;
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
import org.javatrino.ast.Expression.Internal;
import org.javatrino.ast.Expression.Local;
import org.javatrino.ast.Expression.NewArray;
import org.javatrino.ast.Expression.NewObject;
import org.javatrino.ast.Expression.SetField;
import org.javatrino.ast.Expression.TagWithProtocol;
import org.javatrino.ast.Expression.Visitor;
import org.javatrino.ast.Expression.WithEscape;
import org.javatrino.ast.Method;
import org.javatrino.ast.Pattern;
import org.javatrino.ast.Symbol;
import org.javatrino.ast.Test;
import org.neutrino.pib.Module;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.runtime.lookup.CallInfo;

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

    private final String type;

    public LocalAccess(String type) {
      this.type = type;
    }

    public abstract void load(Assembler assm);

    public void store(Assembler assm) {
      assert false;
    }

    @Override
    public String toString() {
      return type;
    }

  }

  private final Map<Symbol, LocalAccess> loaders = new HashMap<Symbol, LocalAccess>();
  private final Assembler assm;
  private int localCount = -1;
  private final Module module;

  private void addLoader(Symbol symbol, LocalAccess loader) {
    if (symbol == null)
      return;
    assert !loaders.containsKey(symbol);
    loaders.put(symbol, loader);
  }

  public BytecodeCompiler(Assembler assm, Module module, final List<Symbol> params,
      final List<Symbol> locals, final Map<Symbol, Expression> rewrites) {
    this.assm = assm;
    this.module = module;
    for (int i = 0; i < params.size(); i++) {
      final int index = i;
      addLoader(params.get(i), new LocalAccess("arg " + i) {
        @Override
        public void load(Assembler assm) {
          assm.argument(params.size() - index);
        }
      });
    }
    this.localCount = locals.size();
    for (int i = 0; i < locals.size(); i++) {
      final int index = i;
      addLoader(locals.get(i), new LocalAccess("local " + i) {
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
        addLoader(rewrite.getKey(), new LocalAccess("rewrite") {
          @Override
          public void load(Assembler assm) {
            rewrite.getValue().accept(BytecodeCompiler.this);
          }
        });
      }
    }
  }

  public static Result compile(Module module, Expression expr, List<Symbol> params,
      Map<Symbol, Expression> rewrites) {
    Assembler assm = new Assembler();
    BytecodeCompiler compiler = new BytecodeCompiler(assm, module, params,
        getLocals(expr), rewrites);
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
      @Override
      public void visitWithEscape(WithEscape that) {
        // Ignore since this will become a separate method.
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
        rewrites.put(sym, eGetField(eLocal(self), getFieldKey(sym)));
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
    } else if (value instanceof RValue) {
      assm.push((RValue) value);
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
    List<CallInfo.ArgumentEntry> entries = new ArrayList<CallInfo.ArgumentEntry>(args.size());
    for (int i = 0; i < args.size(); i++) {
      Argument arg = args.get(i);
      entries.add(new CallInfo.ArgumentEntry(arg.tag, null, i));
    }
    Collections.sort(entries);
    for (int i = 0; i < args.size(); i++) {
      Argument arg = args.get(i);
      arg.value.accept(this);
    }
    assm.call(new CallInfo(1, entries));
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
  public void visitInternal(Internal that) {
    assm.callNative(new Native((String) that.name), that.argc);
  }

  @Override
  public void visitNewObject(NewObject that) {
    assm.newObject();
  }

  @Override
  public void visitNewArray(NewArray that) {
    for (Expression value : that.values)
      value.accept(this);
    assm.newArray(that.values.size());
  }

  @Override
  public void visitWithEscape(WithEscape that) {
    Expression lambda = eAddIntrinsics(eNewObject(), Arrays.asList(
        new Method(
            null,
            Arrays.asList(
                new Pattern(
                    Arrays.<RValue>asList(RString.of("name")),
                    new Test.Eq(RString.of("()")),
                    null),
                new Pattern(
                    Arrays.<RValue>asList(RInteger.get(0)),
                    new Test.Any(),
                    new Symbol()),
                new Pattern(
                    Arrays.<RValue>asList(RInteger.get(1)),
                    new Test.Any(),
                    that.symbol)),
            false,
            that.body,
            null,
            module)));
    lambda.accept(this);
    assm.withEscape();
  }

  public Result getResult() {
    return new Result(assm.getBytecode(), assm.getLiterals(), localCount);
  }

}
