package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.List;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Symbol;
import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.pib.Parameter;
import org.neutrino.syntax.Tree;

public class Compiler {

  public static CodeBundle compile(ModuleBuilder module, Source origin,
      Tree.Expression body, List<Parameter> params) {
    LexicalAnalyzer lexicizer = new LexicalAnalyzer(params);
    body.accept(lexicizer);
    ExpressionGenerator exprgen = new ExpressionGenerator();
    Expression expr = exprgen.generate(body);
    List<Symbol> paramSyms = null;
    if (params != null) {
      paramSyms = new ArrayList<Symbol>();
      for (Parameter param : params)
        paramSyms.add(param.getSymbol());
    }
    return new CodeBundle(origin.getName(), expr, paramSyms, null);
  }

}
