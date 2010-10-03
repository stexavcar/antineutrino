package org.neutrino.compiler;

import java.util.Collections;

import org.neutrino.pib.Assembler;
import org.neutrino.pib.CodeBuilder;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RProtocol;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;

public class ImplicitDeclarationVisitor extends Tree.ExpressionVisitor {

  private final ModuleBuilder module;

  public ImplicitDeclarationVisitor(ModuleBuilder module) {
    this.module = module;
  }

  @Override
  public void visitNew(Tree.New that) {
    RProtocol proto = module.createImplicitProtocol();
    that.bind(proto);
    int index = 0;
    for (Tree.New.Field field : that.getFields()) {
      String name = field.getName();
      CodeBuilder builder = module.createMethod(
          Collections.<Annotation>emptyList(), name,
          Collections.singletonList(new Parameter(name, proto.getName())));
      Assembler assm = builder.getAssembler();
      assm.field(index);
      assm.rethurn();
      index++;
    }
  }

}
