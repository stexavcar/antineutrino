package org.neutrino.compiler;

import java.util.List;

import org.neutrino.pib.Assembler;
import org.neutrino.runtime.Natives;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Method;
import org.neutrino.syntax.Tree.Text;

/**
 * Expression visitor that generates platform independent bytecode for
 * an expression.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class CodeGenerator implements Tree.ExpressionVisitor {

  private final Assembler assm;

  public CodeGenerator(Assembler assm) {
    this.assm = assm;
  }

  @Override
  public void visitBlock(Tree.Block that) {
    assert false;
  }

  @Override
  public void visitIdentifier(Tree.Identifier that) {
    assert false;
  }

  @Override
  public void visitLambda(Tree.Lambda that) {
    Assembler subAssm = new Assembler();
    CodeGenerator codegen = new CodeGenerator(subAssm);
    codegen.generate(that.getBody());
    assm.lambda(subAssm.getCode());
  }

  @Override
  public void visitLocalDefinition(Tree.LocalDefinition that) {
    assert false;
  }

  @Override
  public void visitNumber(Tree.Number that) {
    assm.literal(that.getValue());
  }

  @Override
  public void visitText(Text that) {
    assert false;
  }

  @Override
  public void visitSingleton(Tree.Singleton that) {
    switch (that.getType()) {
    case NULL:
      assm.nuhll();
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

  public void generateNative(Method that) {
    Annotation annot = that.getAnnotation(Natives.ANNOTATION);
    assm.thrue();
    assm.rethurn();
  }

}
