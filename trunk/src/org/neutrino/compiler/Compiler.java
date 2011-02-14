package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.List;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Symbol;
import org.neutrino.pib.Assembler;
import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.pib.Parameter;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Method;

public class Compiler {

  public static CodeBundle compile(ModuleBuilder module,
      Assembler assm, Tree.Expression body) {
    LexicalAnalyzer lexicizer = new LexicalAnalyzer();
    body.accept(lexicizer);
    ImplicitDeclarationVisitor implicitor = new ImplicitDeclarationVisitor(module);
    body.accept(implicitor);
    ExpressionGenerator exprgen = new ExpressionGenerator();
    Expression expr = exprgen.generate(body);
    assm.finalize(expr, null);
    return assm.getCode();
  }

  public static void compileMethod(ModuleBuilder module,
      Assembler assm, Method that) {
    Tree.Expression body = that.getBody();
    LexicalAnalyzer lexicizer = new LexicalAnalyzer(that);
    body.accept(lexicizer);
    ImplicitDeclarationVisitor implicitor = new ImplicitDeclarationVisitor(module);
    body.accept(implicitor);
    ExpressionGenerator exprgen = new ExpressionGenerator();
    Expression expr = exprgen.generate(body);
    List<Symbol> params = new ArrayList<Symbol>();
    for (Parameter param : that.getParameters())
      params.add(param.getSymbol());
    assm.finalize(expr, params);
  }

}
