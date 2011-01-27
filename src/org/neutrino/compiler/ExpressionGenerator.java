package org.neutrino.compiler;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Expression.AddIntrinsics;
import org.javatrino.ast.Expression.Block;
import org.javatrino.ast.Expression.Call;
import org.javatrino.ast.Expression.Call.Argument;
import org.javatrino.ast.Expression.Constant;
import org.javatrino.ast.Expression.Definition;
import org.javatrino.ast.Expression.GetField;
import org.javatrino.ast.Expression.Global;
import org.javatrino.ast.Expression.Local;
import org.javatrino.ast.Expression.NewObject;
import org.javatrino.ast.Expression.SetField;
import org.javatrino.ast.Expression.TagWithProtocol;
import org.javatrino.ast.Method;
import org.javatrino.ast.Pattern;
import org.javatrino.ast.Symbol;
import org.javatrino.ast.Test.Any;
import org.javatrino.ast.Test.Eq;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Assignment;
import org.neutrino.syntax.Tree.Collection;
import org.neutrino.syntax.Tree.Internal;
import org.neutrino.syntax.Tree.Singleton;
import org.neutrino.syntax.Tree.Text;
import org.neutrino.syntax.Tree.With1Cc;

public class ExpressionGenerator extends Tree.ExpressionVisitor<Expression> {

  private static interface IScope {

  }

  private IScope scope;

  private static final Expression ABORT = null;

  private static boolean isAbort(Expression expr) {
    return expr == ABORT;
  }

  @Override
  public Expression visitExpression(Tree.Expression that) {
    assert false : that.getClass();
    return null;
  }

  private static List<Object> asList(Object... args) {
    return Arrays.asList(args);
  }

  private static Method makeGetter(String name, RFieldKey key) {
    Pattern namePattern = new Pattern(asList("name"), new Eq(name), null);
    Symbol self = new Symbol(Symbol.kParameterSymbol);
    Pattern selfPattern = new Pattern(asList(0), new Any(), self);
    Expression body = new GetField(new Local(self), key);
    return new Method(Arrays.asList(namePattern, selfPattern), false, body);
  }

  private static Method makeSetter(String name, RFieldKey key) {
    Pattern namePattern = new Pattern(asList("name"), new Eq(name), null);
    Symbol self = new Symbol(Symbol.kParameterSymbol);
    Pattern selfPattern = new Pattern(asList(0), new Any(), self);
    Symbol value = new Symbol(Symbol.kParameterSymbol);
    Pattern valuePattern = new Pattern(asList(1), new Any(), value);
    Expression body = new SetField(new Local(self), key, new Local(value));
    return new Method(Arrays.asList(namePattern, selfPattern, valuePattern),
        false, body);
  }

  @Override
  public Expression visitNew(Tree.New that) {
    List<Tree.New.Field> fields = that.getFields();
    Symbol obj = new Symbol();
    List<Method> intrinsics = new ArrayList<Method>();
    List<Expression> sets = new ArrayList<Expression>();
    List<Expression> protocols = new ArrayList<Expression>();
    for (Tree.New.Field field : fields) {
      if (!field.hasEagerValue()) {
        return ABORT;
      }
      RFieldKey key = field.getField();
      Expression body = generate(field.getBody());
      if (isAbort(body))
        return ABORT;
      sets.add(new SetField(new Local(obj), key, body));
      intrinsics.add(makeGetter(field.getName(), key));
      intrinsics.add(makeSetter(field.getName() + ":=", key));
    }
    for (Tree.Expression protocol : that.getProtocols()) {
      Expression proto = generate(protocol);
      if (isAbort(proto))
        return ABORT;
      protocols.add(proto);
    }
    Expression result = new NewObject();
    if (!sets.isEmpty()) {
      sets.add(new Local(obj));
      result = new Definition(obj, result, new Block(sets));
    }
    if (!intrinsics.isEmpty())
      result = new AddIntrinsics(result, intrinsics);
    if (!protocols.isEmpty())
      result = new TagWithProtocol(result, protocols);
    return result;
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
    Expression value = generate(that.getValue());
    if (isAbort(value))
      return ABORT;
    Expression body = generate(that.getBody());
    if (isAbort(body))
      return ABORT;
    if (true)
      return ABORT;
    return new Definition(that.getSymbol(), value, body);
  }

  @Override
  public Expression visitBlock(Tree.Block that) {
    List<Expression> exprs = new ArrayList<Expression>();
    for (Tree.Expression source : that.getExpressions()) {
      Expression expr = generate(source);
      if (isAbort(expr))
        return ABORT;
      exprs.add(expr);
    }
    return new Block(exprs);
  }

  @Override
  public Expression visitSingleton(Singleton that) {
    switch (that.getType()) {
    case FALSE:
      return new Constant(false);
    case TRUE:
      return new Constant(true);
    case NULL:
      return new Constant(null);
    default:
      throw new AssertionError(); // Should be LameLanguageDesignError
    }
  }

  @Override
  public Expression visitText(Text that) {
    return ABORT;
  }

  @Override
  public Expression visitWith1Cc(With1Cc that) {
    return ABORT;
  }

  @Override
  public Expression visitCollection(Collection that) {
    return ABORT;
  }

  @Override
  public Expression visitInternal(Internal that) {
    return ABORT;
  }

  @Override
  public Expression visitAssignment(Assignment that) {
    return ABORT;
  }

  @Override
  public Expression visitNumber(Tree.Number that) {
    return new Constant(that.getValue());
  }

  @Override
  public Expression visitCall(Tree.Call that) {
    List<Argument> arguments = new ArrayList<Argument>();
    String name = that.getName();
    arguments.add(new Argument("name", new Constant(name)));
    int index = 0;
    for (Tree.Expression source : that.getArguments()) {
      Expression expr = generate(source);
      if (isAbort(expr))
        return ABORT;
      arguments.add(new Argument(index, expr));
      index++;
    }
    return new Call(arguments);
  }

  public Expression generate(Tree.Expression that) {
    return that.accept(this);
  }

}
