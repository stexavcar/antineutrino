package org.neutrino.compiler;

import static org.javatrino.ast.Expression.StaticFactory.eGetField;
import static org.javatrino.ast.Expression.StaticFactory.eLocal;
import static org.javatrino.ast.Expression.StaticFactory.eSetField;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Symbol;
import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RMethod;
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
        CodeBundle getterCode = new CodeBundle(that.getOrigin().getName(),
            eGetField(eLocal(getSelf.getSymbol()), field.getField()), null,
            null);
        module.createMethod(new RMethod(Collections.<Annotation>emptyList(),
            getterName, Arrays.asList(getSelf), getterCode));
        // Add setter
        String setterName = field.getName() + ":=";
        Parameter setSelf = new Parameter("this", proto.getId(), false);
        Parameter setValue = new Parameter("v", null, false);
        CodeBundle setterCode = new CodeBundle(that.getOrigin().getName(),
            eSetField(eLocal(setSelf.getSymbol()),
                field.getField(), eLocal(setValue.getSymbol())),
            null,
            null);
        module.createMethod(new RMethod(
            Collections.<Annotation>emptyList(),
            setterName,
            Arrays.asList(setSelf, setValue),
            setterCode));
        eagerFieldCount++;
      }
    }
    for (Tree.New.Field field : that.getFields()) {
      if (!field.hasEagerValue()) {
        String name = field.getName();
        Parameter self = new Parameter("this", proto.getId(), false);
        List<Parameter> allParams = new ArrayList<Parameter>();
        allParams.add(self);
        allParams.addAll(field.getParameters());
        List<Symbol> symbols = new ArrayList<Symbol>();
        for (Parameter param : allParams)
          symbols.add(param.getSymbol());
        Expression body = new ExpressionGenerator().generate(field.getBody());
        CodeBundle code = new CodeBundle(that.getOrigin().getName(), body, symbols,
            null);
        module.createMethod(new RMethod(
            Collections.<Annotation>emptyList(),
            name,
            allParams,
            code));
      }
    }
    for (String shuper : that.getProtocolNames())
      module.declareInheritance(proto.getId(), shuper);
    return null;
  }

}
