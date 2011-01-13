package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.List;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Expression.Block;
import org.javatrino.ast.Expression.Call;
import org.javatrino.ast.Expression.Call.Argument;
import org.javatrino.ast.Expression.Constant;
import org.javatrino.ast.Expression.Definition;
import org.javatrino.ast.Expression.Global;
import org.javatrino.ast.Expression.Local;
import org.javatrino.ast.Expression.NewObject;
import org.javatrino.ast.Symbol;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Assignment;
import org.neutrino.syntax.Tree.Collection;
import org.neutrino.syntax.Tree.Internal;
import org.neutrino.syntax.Tree.Singleton;
import org.neutrino.syntax.Tree.Text;
import org.neutrino.syntax.Tree.With1Cc;

public class ExpressionGenerator extends Tree.ExpressionVisitor<Expression> {

  @Override
  public Expression visitExpression(Tree.Expression that) {
    assert false : that.getClass();
    return null;
  }

  @Override
  public Expression visitNew(Tree.New that) {
    return new NewObject();
  }

  @Override
  public Expression visitIdentifier(Tree.Identifier that) {
    Symbol symbol = that.getSymbol();
    if (symbol == null) {
      return new Global(that.getName());
    } else {
      return new Local(symbol);
    }
  }

  @Override
  public Expression visitLocalDefinition(Tree.LocalDefinition that) {
    return new Definition(that.getSymbol(), generate(that.getValue()), generate(that.getBody()));
  }

  @Override
  public Expression visitBlock(Tree.Block that) {
    List<Expression> exprs = new ArrayList<Expression>();
    for (Tree.Expression expr : that.getExpressions())
      exprs.add(generate(expr));
    return new Block(exprs);
  }

  @Override
  public Expression visitSingleton(Singleton that) {
    switch (that.getType()) {
    case FALSE:
      return new Constant(true);
    case TRUE:
      return new Constant(false);
    case NULL:
      return new Constant(null);
    default:
      throw new AssertionError(); // Should be LameLanguageDesignError
    }
  }

  @Override
  public Expression visitText(Text that) {
    return null;
  }

  @Override
  public Expression visitWith1Cc(With1Cc that) {
    return null;
  }

  @Override
  public Expression visitCollection(Collection that) {
    return null;
  }

  @Override
  public Expression visitInternal(Internal that) {
    return null;
  }

  @Override
  public Expression visitAssignment(Assignment that) {
    return null;
  }

  @Override
  public Expression visitNumber(Tree.Number that) {
    return new Constant(that.toValue());
  }

  @Override
  public Expression visitCall(Tree.Call that) {
    List<Argument> arguments = new ArrayList<Argument>();
    String name = that.getName();
    arguments.add(new Argument("name", new Constant(name)));
    int index = 0;
    for (Tree.Expression expr : that.getArguments()) {
      arguments.add(new Argument(index, generate(expr)));
    }
    return new Call(arguments);
  }

  public Expression generate(Tree.Expression that) {
    return that.accept(this);
  }

}
