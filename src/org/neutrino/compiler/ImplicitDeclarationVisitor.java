package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.neutrino.pib.Assembler;
import org.neutrino.pib.CodeBuilder;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RProtocol;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;

public class ImplicitDeclarationVisitor extends Tree.ExpressionVisitor<Void> {

  private final ModuleBuilder module;

  public ImplicitDeclarationVisitor(ModuleBuilder module) {
    this.module = module;
  }

  @Override
  public Void visitNew(Tree.New that) {
    RProtocol proto = module.createImplicitProtocol(that.getOrigin(),
        that.getDisplayName());
    that.bind(proto);
    int eagerFieldCount = 0;
    for (Tree.New.Field field : that.getFields()) {
      field.getBody().accept(this);
      if (field.hasEagerValue()) {
        // Add getter
        String getterName = field.getName();
        CodeBuilder getterBuilder = module.createMethod(
            that.getOrigin(),
            Collections.<Annotation>emptyList(),
            getterName,
            Arrays.asList(new Parameter("this", proto.getId(), false)));
        Assembler getterAssm = getterBuilder.getAssembler();
        getterAssm.doGetter(field.getField());
        getterAssm.rethurn();
        getterAssm.finalize(0, 0, null, null);
        // Add setter
        String setterName = field.getName() + ":=";
        CodeBuilder setterBuilder = module.createMethod(
            that.getOrigin(),
            Collections.<Annotation>emptyList(),
            setterName,
            Arrays.asList(
                new Parameter("this", proto.getId(), false),
                new Parameter("v", null, false)));
        Assembler setterAssm = setterBuilder.getAssembler();
        setterAssm.doSetter(field.getField());
        setterAssm.rethurn();
        setterAssm.finalize(0, 0, null, null);
        eagerFieldCount++;
      }
    }
    for (Tree.New.Field field : that.getFields()) {
      if (!field.hasEagerValue()) {
        String name = field.getName();
        Parameter param = new Parameter("this", proto.getId(), false);
        List<Parameter> allParams = new ArrayList<Parameter>();
        allParams.add(param);
        allParams.addAll(field.getParameters());
        CodeBuilder builder = module.createMethod(
            that.getOrigin(),
            Collections.<Annotation>emptyList(),
            name,
            allParams);
        Assembler assm = builder.getAssembler();
        CodeGenerator codegen = new CodeGenerator(module.getUniverse(), assm);
        int rootOffset = codegen.generate(field.getBody());
        assm.finalize(field.getLocalCount(), rootOffset, null, null);
      }
    }
    for (String shuper : that.getProtocolNames())
      module.declareInheritance(proto.getId(), shuper);
    return null;
  }

}
