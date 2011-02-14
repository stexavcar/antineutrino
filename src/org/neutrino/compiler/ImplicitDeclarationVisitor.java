package org.neutrino.compiler;

import static org.javatrino.ast.Expression.StaticFactory.eGetField;
import static org.javatrino.ast.Expression.StaticFactory.eLocal;
import static org.javatrino.ast.Expression.StaticFactory.eSetField;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.javatrino.ast.Expression;
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
        Parameter getSelf = new Parameter("this", proto.getId(), false);
        CodeBuilder getterBuilder = module.createMethod(
            that.getOrigin(),
            Collections.<Annotation>emptyList(),
            getterName,
            Arrays.asList(getSelf));
        Assembler getterAssm = getterBuilder.getAssembler();
        getterAssm.finalize(eGetField(eLocal(getSelf.getSymbol()), field.getField()), null);
        // Add setter
        String setterName = field.getName() + ":=";
        Parameter setSelf = new Parameter("this", proto.getId(), false);
        Parameter setValue = new Parameter("v", null, false);
        CodeBuilder setterBuilder = module.createMethod(
            that.getOrigin(),
            Collections.<Annotation>emptyList(),
            setterName,
            Arrays.asList(setSelf, setValue));
        Assembler setterAssm = setterBuilder.getAssembler();
        setterAssm.finalize(eSetField(eLocal(setSelf.getSymbol()),
            field.getField(), eLocal(setValue.getSymbol())), null);
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
        Expression body = new ExpressionGenerator().generate(field.getBody());
        assm.finalize(body, null);
      }
    }
    for (String shuper : that.getProtocolNames())
      module.declareInheritance(proto.getId(), shuper);
    return null;
  }

}
