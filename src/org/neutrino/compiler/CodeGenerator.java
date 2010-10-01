package org.neutrino.compiler;

import org.neutrino.pib.Assembler;
import org.neutrino.runtime.Native;
import org.neutrino.runtime.RString;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Definition;
import org.neutrino.syntax.Tree.Expression;
import org.neutrino.syntax.Tree.Method;
import org.neutrino.syntax.Tree.Text;

import java.util.List;

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
    that.getSymbol().emit(assm);
  }

  @Override
  public void visitLambda(Tree.Lambda that) {
    Assembler subAssm = new Assembler();
    CodeGenerator codegen = new CodeGenerator(subAssm);
    codegen.generate(that.getBody());
    List<Symbol> captured = that.getCaptured();
    subAssm.setLocalCount(that.getLocalCount());
    for (Symbol symbol : captured)
      symbol.emit(assm);
    assm.lambda(subAssm.getCode(), captured.size());
  }

  public void generateNativeLambda(Definition that) {
    Assembler subAssm = new Assembler();
    Annotation annot = that.getAnnotation(Native.ANNOTATION);
    String key = ((RString) annot.getArgument(0)).getValue();
    Native method = Native.get(key);
    Tree.Lambda lambda = (Tree.Lambda) that.getValue();
    subAssm.callNative(method, lambda.getParameters().size() + 1);
    subAssm.rethurn();
    assm.lambda(subAssm.getCode(), 0);
    assm.rethurn();
  }

  @Override
  public void visitLocalDefinition(Tree.LocalDefinition that) {
    that.getValue().accept(this);
    int index = that.getSymbol().getIndex();
    assm.storeLocal(index);
    that.getBody().accept(this);
  }

  @Override
  public void visitNumber(Tree.Number that) {
    assm.literal(that.getValue());
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