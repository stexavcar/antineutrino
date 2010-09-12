package org.neutrino.pib;

import org.neutrino.syntax.Tree;


public class CodeBuilder<Ast extends Tree.Declaration> {

  private final Ast ast;
  private final Assembler assm;

  public CodeBuilder(Ast ast) {
    this.ast = ast;
    this.assm = new Assembler();
  }

  public Ast getSyntax() {
    return this.ast;
  }

  public Assembler getAssembler() {
    return this.assm;
  }

  public Binding getBinding() {
    return new Binding(ast.getAnnotations(), assm.getCode());
  }

}
