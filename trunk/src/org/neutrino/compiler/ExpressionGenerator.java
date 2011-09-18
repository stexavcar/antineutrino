package org.neutrino.compiler;

import static org.javatrino.ast.Expression.StaticFactory.*;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Expression.Call;
import org.javatrino.ast.Expression.NewDictionary;
import org.javatrino.ast.Method;
import org.javatrino.ast.Pattern;
import org.javatrino.ast.Symbol;
import org.javatrino.ast.Test;
import org.javatrino.ast.Test.Any;
import org.javatrino.ast.Test.Eq;
import org.javatrino.ast.Test.Is;
import org.neutrino.pib.Module;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RBoolean;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RNull;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;
import org.neutrino.syntax.Annotation;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Collection;
import org.neutrino.syntax.Tree.Collection.Flavor;
import org.neutrino.syntax.Tree.Internal;

public class ExpressionGenerator extends Tree.ExpressionVisitor<Expression> {

  private final Module module;

  public ExpressionGenerator(Module module) {
    this.module = module;
  }

  @Override
  public Expression visitExpression(Tree.Expression that) {
    assert false : that.getClass();
    return null;
  }

  private static List<RValue> asList(RValue... args) {
    return Arrays.asList(args);
  }

  private Method makeGetter(String name, RFieldKey key) {
    Pattern namePattern = new Pattern(asList(RString.of("name")),
        new Eq(RString.of(name)), null);
    Symbol self = new Symbol(Symbol.kParameterSymbol);
    Pattern selfPattern = new Pattern(asList(RInteger.get(0)), new Any(), self);
    Expression body = eGetField(eLocal(self), key);
    return new Method(null, Arrays.asList(namePattern, selfPattern), false, body,
        null, module, null);
  }

  private Method makeSetter(String name, RFieldKey key) {
    Pattern namePattern = new Pattern(asList(RString.of("name")),
        new Eq(RString.of(name)), null);
    Symbol self = new Symbol(Symbol.kParameterSymbol);
    Pattern selfPattern = new Pattern(asList(RInteger.get(0)), new Any(), self);
    Symbol value = new Symbol(Symbol.kParameterSymbol);
    Pattern valuePattern = new Pattern(asList(RInteger.get(1)), new Any(), value);
    Expression body = eSetField(eLocal(self), key, eLocal(value));
    return new Method(null, Arrays.asList(namePattern, selfPattern, valuePattern),
        false, body, null, module, null);
  }

  @Override
  public Expression visitNew(Tree.New that) {
    List<Tree.New.Field> fields = that.getFields();
    Symbol obj = new Symbol();
    List<Method> intrinsics = new ArrayList<Method>();
    List<Expression> sets = new ArrayList<Expression>();
    List<Expression> protocols = new ArrayList<Expression>();
    for (Tree.New.Field field : fields) {
      Expression body = generate(field.getBody());
      if (field.hasEagerValue()) {
        RFieldKey key = field.getField();
        sets.add(eSetField(eLocal(obj), key, body));
        intrinsics.add(makeGetter(field.getName(), key));
        intrinsics.add(makeSetter(field.getName() + ":=", key));
      } else {
        Pattern namePattern = new Pattern(asList(RString.of("name")), new Eq(RString.of(field.getName())), null);
        Symbol self = new Symbol(Symbol.kImplicitThis);
        Pattern selfPattern = new Pattern(asList(RInteger.get(0)), new Any(), self);
        List<Pattern> patterns = new ArrayList<Pattern>();
        patterns.add(namePattern);
        patterns.add(selfPattern);
        int index = 1;
        for (Parameter param : field.getParameters()) {
          Test test;
          if ("Object".equals(param.type)) {
            test = new Any();
          } else {
            test = new Is(param.ensureProtocol(module));
          }
          patterns.add(new Pattern(asList(RInteger.get(index)), test, param.getSymbol()));
          index++;
        }
        Method method = new Method(null, patterns, false, body, null, module, null);
        intrinsics.add(method);
      }
    }
    for (Tree.Expression protocol : that.getProtocols()) {
      Expression proto = generate(protocol);
      protocols.add(proto);
    }
    Expression result = eNewObject();
    if (!sets.isEmpty()) {
      sets.add(eLocal(obj));
      result = eDefinition(obj, result, eBlock(sets), Annotation.NONE);
    }
    if (!intrinsics.isEmpty())
      result = eAddIntrinsics(result, intrinsics);
    if (!protocols.isEmpty())
      result = eTagWithProtocol(result, protocols);
    return result;
  }

  @Override
  public Expression visitIdentifier(Tree.Identifier that) {
    Symbol symbol = that.getSymbol();
    if (symbol == null) {
      return eGlobal(that.getName());
    } else if (that.isReference()) {
      return eCall(eArgument(RString.of("name"), eConstant(RString.of("get"))),
          eArgument(RInteger.get(0), eLocal(symbol)));
    } else {
      return eLocal(symbol);
    }
  }

  @Override
  public Expression visitAssignment(Tree.Assignment that) {
    Expression value = generate(that.getValue());
    return eCall(eArgument(RString.of("name"), eConstant(RString.of("set"))),
        eArgument(RInteger.get(0), eLocal(that.getSymbol())),
        eArgument(RInteger.get(1), value));
  }

  @Override
  public Expression visitLocalDefinition(Tree.LocalDefinition that) {
    Expression value = generate(that.getValue());
    Expression body = generate(that.getBody());
    if (that.isReference()) {
      value = eCall(eArgument(RString.of("name"), eConstant(RString.of("new"))),
          eArgument(RInteger.get(0), eGlobal(RString.of("Ref"))),
          eArgument(RInteger.get(1), value));
    }

    return eDefinition(that.getSymbol(), value, body,
        Source.resolveAnnotations(module, that.getAnnotations()));
  }

  @Override
  public Expression visitBlock(Tree.Block that) {
    List<Expression> exprs = new ArrayList<Expression>();
    for (Tree.Expression source : that.getExpressions()) {
      Expression expr = generate(source);
      exprs.add(expr);
    }
    return eBlock(exprs);
  }

  @Override
  public Expression visitSingleton(Tree.Singleton that) {
    switch (that.getType()) {
    case FALSE:
      return eConstant(RBoolean.getFalse());
    case TRUE:
      return eConstant(RBoolean.getTrue());
    case NULL:
      return eConstant(RNull.getInstance());
    default:
      throw new AssertionError(); // Should be LameLanguageDesignError
    }
  }

  @Override
  public Expression visitText(Tree.Text that) {
    return eConstant(RString.of(that.getValue()));
  }

  @Override
  public Expression visitWithEscape(Tree.WithEscape that) {
    Expression body = that.getBody().accept(this);
    return eWithEscape(that.getSymbol(), body);
  }

  @Override
  public Expression visitCollection(Tree.Collection that) {
    if (that.getFlavor() == Flavor.DICTIONARY) {
      List<NewDictionary.Entry> elms = new ArrayList<NewDictionary.Entry>();
      for (Collection.Entry entry : that.getEntries()) {
        Expression key = entry.key.accept(this);
        Expression value = entry.value.accept(this);
        elms.add(new NewDictionary.Entry(key, value));
      }
      return eNewDictionary(elms);
    } else {
      List<Expression> elms = new ArrayList<Expression>();
      for (Collection.Entry entry : that.getEntries()) {
        Expression value = entry.value.accept(this);
        elms.add(value);
      }
      return eNewObjectArray(elms);
    }
  }

  @Override
  public Expression visitInternal(Internal that) {
    return eInternal(that.getName(), that.getArgumentCount());
  }

  @Override
  public Expression visitNumber(Tree.Number that) {
    return eConstant(RInteger.get(that.getValue()));
  }

  @Override
  public Expression visitCall(Tree.Call that) {
    List<Call.Argument> arguments = new ArrayList<Call.Argument>();
    String name = that.getName();
    arguments.add(eArgument(RString.of("name"), eConstant(RString.of(name))));
    int index = 0;
    for (Tree.Argument source : that.getArguments()) {
      Expression expr = generate(source.getValue());
      arguments.add(eArgument(source.getKeyword(), expr));
      index++;
    }
    return eCall(arguments);
  }

  public Expression generate(Tree.Expression that) {
    return that.accept(this);
  }

}
