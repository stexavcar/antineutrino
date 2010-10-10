package org.neutrino.compiler;

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
public class CodeGenerator extends Tree.ExpressionVisitor {

  private final Assembler assm;

  public CodeGenerator(Assembler assm) {
    this.assm = assm;
  }

  @Override
  public void visitExpression(Expression that) {
    assert false;
  }

  @Override
  public void visitBlock(Tree.Block that) {
    List<Expression> exprs = that.getExpressions();
    for (int i = 0; i < exprs.size(); i++) {
      if (i > 0) {
        assm.pop();
      }
      exprs.get(i).accept(this);
    }
  }

  @Override
  public void visitIdentifier(Tree.Identifier that) {
    that.getSymbol().emitLoad(assm);
    if (that.getSymbol().isReference())
      assm.call("get", 1);
  }

  @Override
  public void visitAssignment(Tree.Assignment that) {
    assert that.getSymbol().isReference();
    that.getSymbol().emitLoad(assm);
    that.getValue().accept(this);
    assm.call("set", 2);
  }

  @Override
  public void visitLocalDefinition(Tree.LocalDefinition that) {
    if (that.isReference())
      assm.global("new_ref");
    that.getValue().accept(this);
    if (that.isReference())
      assm.call("()", 2);
    int index = that.getSymbol().getIndex();
    assm.storeLocal(index);
    that.getBody().accept(this);
  }

  @Override
  public void visitNumber(Tree.Number that) {
    assm.push(RInteger.get(that.getValue()));
  }

  @Override
  public void visitText(Text that) {
    assm.push(new RString(that.getValue()));
  }

  @Override
  public void visitSingleton(Tree.Singleton that) {
    switch (that.getType()) {
    case NULL:
      assm.nuhll();
      break;
    case TRUE:
      assm.thrue();
      break;
    case FALSE:
      assm.fahlse();
      break;
    default:
      assert false;
    }
  }

  @Override
  public void visitCall(Tree.Call that) {
    List<Tree.Expression> args = that.getArguments();
    int argc = args.size();
    for (int i = 0; i < argc; i++) {
      Tree.Expression arg = args.get(i);
      arg.accept(this);
    }
    assm.call(that.getName(), argc);
  }

  @Override
  public void visitWith1Cc(With1Cc that) {
    that.getCallback().accept(this);
    assm.with1Cc();
  }

  @Override
  public void visitNew(New that) {
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
  }

  @Override
  public void visitCollection(Collection that) {
    List<Tree.Expression> values = that.getValues();
    for (Tree.Expression value : values)
      value.accept(this);
    assm.newArray(values.size());
  }

  @Override
  public void visitInternal(Internal that) {
    assm.callNative(Native.get(that.getName()), that.getArgumentCount());
  }

  public void generate(Tree.Expression value) {
    value.accept(this);
    assm.rethurn();
  }

  public void generateNativeMethod(Method that) {
    Annotation annot = that.getAnnotation(Native.ANNOTATION);
    String key = ((RString) annot.getArgument(0)).getValue();
    Native method = Native.get(key);
    assm.callNative(method, that.getParameters().size());
    assm.rethurn();
  }

}
