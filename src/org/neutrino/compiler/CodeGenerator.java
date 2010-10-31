package org.neutrino.compiler;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.neutrino.pib.Assembler;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RString;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Collection;
import org.neutrino.syntax.Tree.Expression;
import org.neutrino.syntax.Tree.Internal;
import org.neutrino.syntax.Tree.Method;
import org.neutrino.syntax.Tree.New;
import org.neutrino.syntax.Tree.Text;
import org.neutrino.syntax.Tree.With1Cc;

/**
 * Expression visitor that generates platform independent bytecode for
 * an expression.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class CodeGenerator extends Tree.ExpressionVisitor<Integer> {

  private final CompilerUniverse universe;
  private final Assembler assm;

  public CodeGenerator(CompilerUniverse universe, Assembler assm) {
    this.universe = universe;
    this.assm = assm;
  }

  @Override
  public Integer visitExpression(Expression that) {
    assert false;
    return null;
  }

  @Override
  public Integer visitBlock(Tree.Block that) {
    List<Expression> exprs = that.getExpressions();
    for (int i = 0; i < exprs.size(); i++) {
      if (i > 0) {
        assm.pop();
      }
      exprs.get(i).accept(this);
    }
    // TODO
    return -2;
  }

  @Override
  public Integer visitIdentifier(Tree.Identifier that) {
    int result = that.getSymbol().emitLoad(assm);
    if (that.getSymbol().isReference())
      result = assm.call("get", Arrays.asList(result));
    return result;
  }

  @Override
  public Integer visitAssignment(Tree.Assignment that) {
    assert that.getSymbol().isReference();
    int ref = that.getSymbol().emitLoad(assm);
    int value = that.getValue().accept(this);
    return assm.call("set", Arrays.asList(ref, value));
  }

  @Override
  public Integer visitLocalDefinition(Tree.LocalDefinition that) {
    int ref = -1;
    if (that.isReference())
      ref = assm.global("Ref");
    int value = that.getValue().accept(this);
    if (that.isReference())
      assm.call("new", Arrays.asList(ref, value));
    int index = that.getSymbol().getIndex();
    assm.storeLocal(index);
    that.getBody().accept(this);
    // TODO
    return -3;
  }

  @Override
  public Integer visitNumber(Tree.Number that) {
    return assm.push(RInteger.get(that.getValue()));
  }

  @Override
  public Integer visitText(Text that) {
    return assm.push(new RString(that.getValue()));
  }

  @Override
  public Integer visitSingleton(Tree.Singleton that) {
    switch (that.getType()) {
    case NULL:
      return assm.nuhll();
    case TRUE:
      return assm.thrue();
    case FALSE:
      return assm.fahlse();
    default:
      assert false;
      return -4;
    }
  }

  private Tree.Declaration getIntrinsicTarget(Tree.Call call) {
    for (Tree.Expression arg : call.getArguments()) {
      Tree.Declaration origin = arg.getStaticValue(universe);
      if (origin != null) {
        if (origin.getAnnotation(Intrinsic.ANNOTATION) != null)
          return origin;
      }
    }
    return null;
  }

  @Override
  public Integer visitCall(Tree.Call that) {
    Tree.Declaration intrinsicTarget = getIntrinsicTarget(that);
    if (intrinsicTarget != null) {
      String holder = intrinsicTarget.getName();
      Tree.Method method = universe.findMethod(holder, that.getName());
      if (method != null) {
        Annotation annot = method.getAnnotation(Intrinsic.ANNOTATION);
        if (annot != null) {
          String id = ((RString) annot.getArgument(0)).getValue();
          Intrinsic handler = Intrinsic.get(id);
          if (handler != null && handler.isApplicable(that)) {
            handler.generate(that, this);
          }
        }
      }
    }
    List<Tree.Expression> args = that.getArguments();
    List<Integer> offsets = new ArrayList<Integer>();
    int argc = args.size();
    for (int i = 0; i < argc; i++) {
      Tree.Expression arg = args.get(i);
      int offset = arg.accept(this);
      offsets.add(offset);
    }
    return assm.call(that.getName(), offsets);
  }

  @Override
  public Integer visitWith1Cc(With1Cc that) {
    that.getCallback().accept(this);
    return assm.with1Cc();
  }

  @Override
  public Integer visitNew(New that) {
    List<Tree.New.Field> fields = that.getFields();
    int outerCount = 0;
    for (Tree.New.Field field : fields) {
      if (field.hasEagerValue()) {
        field.getBody().accept(this);
        outerCount++;
      }
    }
    for (Symbol symbol : that.getCaptures()) {
      symbol.emitLoad(assm);
      outerCount++;
    }
    assm.nhew(that.getProtocol(), outerCount);
    // TODO
    return -5;
  }

  @Override
  public Integer visitCollection(Collection that) {
    List<Tree.Expression> values = that.getValues();
    for (Tree.Expression value : values)
      value.accept(this);
    assm.newArray(values.size());
    // TODO
    return -6;
  }

  @Override
  public Integer visitInternal(Internal that) {
    assm.callNative(Native.get(that.getName()), that.getArgumentCount());
    // TODO
    return -7;
  }

  public Integer generate(Tree.Expression value) {
    Integer result = value.accept(this);
    assm.rethurn();
    return result;
  }

  public void generateNativeMethod(Method that) {
    Annotation annot = that.getAnnotation(Native.ANNOTATION);
    String key = ((RString) annot.getArgument(0)).getValue();
    Native method = Native.get(key);
    assm.callNative(method, that.getParameters().size());
    assm.rethurn();
  }

}
