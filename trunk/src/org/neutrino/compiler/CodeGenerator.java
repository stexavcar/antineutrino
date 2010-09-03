package org.neutrino.compiler;

import org.neutrino.pib.Assembler;
import org.neutrino.syntax.Tree;

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
    assm.lambda();
    that.getBody().accept(this);
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
  public void visitSingleton(Tree.Singleton that) {
    assert false;
  }

}
