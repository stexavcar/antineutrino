package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

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
    RProtocol proto = module.createImplicitProtocol(that.getDisplayName());
    that.bind(proto);
    int eagerFieldCount = 0;
    for (Tree.New.Field field : that.getFields()) {
      field.getBody().accept(this);
      if (field.hasEagerValue()) {
        String name = field.getName();
        CodeBuilder builder = module.createMethod(
            Collections.<Annotation>emptyList(), name,
            Collections.singletonList(new Parameter("this", proto.getId())));
        Assembler assm = builder.getAssembler();
        assm.field(eagerFieldCount);
        assm.rethurn();
        assm.setLocalCount(0);
        eagerFieldCount++;
      }
    }
    for (Tree.New.Field field : that.getFields()) {
      if (!field.hasEagerValue()) {
        String name = field.getName();
        Parameter param = new Parameter("this", proto.getId());
        List<Parameter> allParams = new ArrayList<Parameter>();
        allParams.add(param);
        allParams.addAll(field.getParameters());
        CodeBuilder builder = module.createMethod(
            Collections.<Annotation>emptyList(), name,
            allParams);
        Assembler assm = builder.getAssembler();
        CodeGenerator codegen = new CodeGenerator(assm);
        codegen.generate(field.getBody());
        assm.setLocalCount(field.getLocalCount());
      }
    }
    for (String shuper : that.getProtocols())
      module.declareInheritance(proto.getId(), shuper);
  }

}
