package org.neutrino.syntax;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.neutrino.compiler.Symbol;
import org.neutrino.compiler.Symbol.LocalSymbol;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RInteger;
import org.neutrino.runtime.RNull;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RString;
import org.neutrino.runtime.RValue;

/**
 * Neutrino syntax trees.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Tree {

  public static class Unit extends Tree {

    private final List<Declaration> decls;

    public Unit(List<Declaration> decls) {
      this.decls = decls;
    }

    public List<Declaration> getDeclarations() {
      return this.decls;
    }

    @Override
    public String toString() {
      return Tree.toString("unit", decls);
    }

  }

  public interface DeclarationVisitor {

    void visitDefinition(Definition that);

    void visitProtocol(Protocol that);

    void visitMethodDefinition(Method that);

    void visitInheritance(Inheritance that);

  }

  public static abstract class Declaration extends Tree {

    public abstract void accept(DeclarationVisitor visitor);

    public abstract List<Annotation> getAnnotations();

    public Annotation getAnnotation(String tag) {
      for (Annotation annot : getAnnotations()) {
        if (annot.getTag().equals(tag))
          return annot;
      }
      return null;
    }

  }

  public static class Definition extends Declaration {

    private final List<Annotation> annots;
    private final String name;
    private final Expression value;

    public Definition(List<Annotation> annots, String name, Expression value) {
      this.annots = annots;
      this.name = name;
      this.value = value;
    }

    public String getName() {
      return this.name;
    }

    public Expression getValue() {
      return this.value;
    }

    @Override
    public List<Annotation> getAnnotations() {
      return this.annots;
    }

    @Override
    public void accept(DeclarationVisitor visitor) {
      visitor.visitDefinition(this);
    }

    @Override
    public String toString() {
      String an = annotationsToString(getAnnotations());
      return "(def " + an + name + " " + value + ")";
    }

  }

  public static class Protocol extends Declaration {

    private final List<Annotation> annots;
    private final String name;

    public Protocol(List<Annotation> annots, String name) {
      this.annots = annots;
      this.name = name;
    }

    public String getName() {
      return this.name;
    }

    @Override
    public List<Annotation> getAnnotations() {
      return this.annots;
    }

    @Override
    public void accept(DeclarationVisitor visitor) {
      visitor.visitProtocol(this);
    }

    @Override
    public String toString() {
      String an = annotationsToString(getAnnotations());
      return "(protocol " + an + name + ")";
    }

  }

  public static class Inheritance extends Declaration {

    private final List<Annotation> annots;
    private final String name;
    private final String parent;

    public Inheritance(List<Annotation> annots, String name, String parent) {
      this.annots = annots;
      this.name = name;
      this.parent = parent;
    }

    public String getSub() {
      return name;
    }

    public String getSuper() {
      return parent;
    }

    @Override
    public void accept(DeclarationVisitor visitor) {
      visitor.visitInheritance(this);
    }

    @Override
    public List<Annotation> getAnnotations() {
      return annots;
    }

  }

  public static class Method extends Declaration {

    private final List<Annotation> annots;
    private final String name;
    private final List<Parameter> params;
    private final Expression body;

    public Method(List<Annotation> annots, String name, List<Parameter> params,
        Expression body) {
      this.annots = annots;
      this.name = name;
      this.params = params;
      this.body = body;
    }

    public List<Parameter> getParameters() {
      return this.params;
    }

    public String getName() {
      return this.name;
    }

    @Override
    public void accept(DeclarationVisitor visitor) {
      visitor.visitMethodDefinition(this);
    }

    @Override
    public List<Annotation> getAnnotations() {
      return this.annots;
    }

    public Expression getBody() {
      return this.body;
    }

    @Override
    public String toString() {
      String an = annotationsToString(this.annots);
      return "(method " + an + Tree.toString(this.name, params) + " " + body + ")";
    }

  }

  public static abstract class Expression extends Tree {

    public abstract void accept(ExpressionVisitor visitor);

    public abstract void traverse(ExpressionVisitor visitor);

    public RValue toValue() {
      assert false;
      return RNull.getInstance();
    }

  }

  public static class ExpressionVisitor {

    public void visitExpression(Expression that) {
      that.traverse(this);
    }

    public void visitLocalDefinition(LocalDefinition that) {
      visitExpression(that);
    }

    public void visitBlock(Block that) {
      visitExpression(that);
    }

    public void visitSingleton(Singleton that) {
      visitExpression(that);
    }

    public void visitNumber(Number that) {
      visitExpression(that);
    }

    public void visitIdentifier(Identifier that) {
      visitExpression(that);
    }

    public void visitCall(Call that) {
      visitExpression(that);
    }

    public void visitText(Text that) {
      visitExpression(that);
    }

    public void visitNew(New that) {
      visitExpression(that);
    }

    public void visitInternal(Internal that) {
      visitExpression(that);
    }

    public void visitWith1Cc(With1Cc that) {
      visitExpression(that);
    }

    public void visitAssignment(Assignment that) {
      visitExpression(that);
    }

  }

  public static class Identifier extends Expression {

    private final String name;
    private Symbol symbol;

    public Identifier(String name) {
      this.name = name;
    }

    public String getName() {
      return name;
    }

    public void bind(Symbol symbol) {
      assert this.symbol == null;
      this.symbol = symbol;
    }

    public Symbol getSymbol() {
      assert this.symbol != null;
      return this.symbol;
    }

    @Override
    public String toString() {
      return "$" + name;
    }

    @Override
    public void traverse(ExpressionVisitor visitor) { }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitIdentifier(this);
    }

  }

  public static class Assignment extends Expression {

    private final String name;
    private final Tree.Expression value;
    private Symbol symbol;

    public Assignment(String name, Tree.Expression value) {
      this.name = name;
      this.value = value;
    }

    public Symbol getSymbol() {
      assert symbol != null;
      return symbol;
    }

    public String getName() {
      return this.name;
    }

    public Tree.Expression getValue() {
      return this.value;
    }

    public void bind(Symbol symbol) {
      assert this.symbol == null;
      this.symbol = symbol;
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitAssignment(this);
    }

    @Override
    public void traverse(ExpressionVisitor visitor) {
      value.accept(visitor);
    }

  }

  public static class Number extends Expression {

    private final int value;

    public Number(int value) {
      this.value = value;
    }

    public int getValue() {
      return this.value;
    }

    @Override
    public String toString() {
      return "#" + value;
    }

    @Override
    public RValue toValue() {
      return new RInteger(value);
    }

    @Override
    public void traverse(ExpressionVisitor visitor) { }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitNumber(this);
    }

  }

  public static class Text extends Expression {

    private final String value;

    public Text(String value) {
      this.value = value;
    }

    public String getValue() {
      return this.value;
    }

    @Override
    public void traverse(ExpressionVisitor visitor) { }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitText(this);
    }

    @Override
    public RValue toValue() {
      return new RString(value);
    }

    @Override
    public String toString() {
      return "#'" + value + "'";
    }

  }

  public static class Internal extends Expression {

    private final String name;
    private int argc = -1;

    public Internal(String name) {
      this.name = name;
    }

    public void setArgumentCount(int value) {
      assert argc == -1;
      this.argc = value;
    }

    public int getArgumentCount() {
      assert argc != -1;
      return argc;
    }

    public String getName() {
      return name;
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitInternal(this);
    }

    @Override
    public void traverse(ExpressionVisitor visitor) {
      // ignore
    }

  }

  public static class Lambda {

    public static Expression create(List<Parameter> params, Expression body) {
      New.Field call = new New.Field(params, "()", body, false);
      return new New(Collections.singletonList(call),
          Collections.<String>emptyList());
    }

    public static Expression createCall(Expression fun, List<Expression> args) {
      List<Expression> allArgs = new ArrayList<Expression>();
      allArgs.add(fun);
      allArgs.addAll(args);
      return new Call("()", allArgs);
    }

  }

  public static class If {

    public static Expression create(Expression cond, Expression thenPart,
        Tree.Expression elsePart) {
      return Lambda.createCall(
          Lambda.createCall(
              new Identifier("select"),
              Arrays.asList(
                  cond,
                  Lambda.create(Collections.<Parameter>emptyList(), thenPart),
                  Lambda.create(Collections.<Parameter>emptyList(), elsePart))),
          Collections.<Expression>emptyList());
    }

  }

  public static class With1Cc extends Expression {

    private final Expression callback;

    private With1Cc(Expression callback) {
      this.callback = callback;
    }

    public Expression getCallback() {
      return callback;
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitWith1Cc(this);
    }

    @Override
    public void traverse(ExpressionVisitor visitor) {
      callback.accept(visitor);
    }

    public static Expression create(String name, Expression body) {
      Parameter param = new Parameter(name, "Object");
      Expression lambda = Lambda.create(Collections.singletonList(param),
          body);
      return new With1Cc(lambda);
    }

  }

  public static class Call extends Expression {

    private final String name;
    private final List<Expression> args;

    public Call(String name, List<Expression> args) {
      this.name = name;
      this.args = args;
    }

    @Override
    public void traverse(ExpressionVisitor visitor) {
      for (Expression arg : args)
        arg.accept(visitor);
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitCall(this);
    }

    public List<Expression> getArguments() {
      return this.args;
    }

    public String getName() {
      return this.name;
    }

    @Override
    public String toString() {
      StringBuilder result = new StringBuilder();
      result.append("(.").append(name);
      for (Expression arg : args)
        result.append(" ").append(arg);
      return result.append(")").toString();
    }

  }

  public static class Singleton extends Expression {

    public enum Type {
      NULL("#n"), TRUE("#t"), FALSE("#f");
      private final String name;
      private Type(String name) { this.name = name; }
      @Override public String toString() { return this.name; }
    }

    private final Type type;

    public Singleton(Type type) {
      this.type = type;
    }

    public Type getType() {
      return this.type;
    }

    @Override
    public String toString() {
      return type.toString();
    }

    @Override
    public void traverse(ExpressionVisitor visitor) { }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitSingleton(this);
    }

  }

  public static class New extends Expression {

    public static class Field {

      private final List<Parameter> params;
      private final String name;
      private final Expression body;
      private final boolean hasEagerValue;
      private int localCount = -1;

      public Field(List<Parameter> params, String name, Expression body,
          boolean hasEagerValue) {
        this.params = params;
        this.name = name;
        this.body = body;
        this.hasEagerValue = hasEagerValue;
      }

      public String getName() {
        return name;
      }

      public Expression getBody() {
        return body;
      }

      public List<Parameter> getParameters() {
        return params;
      }

      public boolean hasEagerValue() {
        return this.hasEagerValue;
      }

      public void setLocalCount(int localCount) {
        assert this.localCount == -1;
        this.localCount = localCount;
      }

      public int getLocalCount() {
        assert this.localCount != -1;
        return this.localCount;
      }

    }

    private final List<Field> fields;
    private final List<String> protocols;
    private RProtocol protocol;
    private List<Symbol> captures;

    public New(List<Field> fields, List<String> protocols) {
      this.fields = fields;
      this.protocols = protocols;
    }

    public List<String> getProtocols() {
      return protocols;
    }

    public void setCaptures(List<Symbol> captures) {
      assert this.captures == null;
      this.captures = captures;
    }

    public List<Symbol> getCaptures() {
      assert this.captures != null;
      return this.captures;
    }

    public RProtocol getProtocol() {
      assert protocol != null;
      return protocol;
    }

    public List<Field> getFields() {
      return fields;
    }

    public void bind(RProtocol proto) {
      assert protocol == null;
      this.protocol = proto;
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitNew(this);
    }

    @Override
    public void traverse(ExpressionVisitor visitor) {
      for (Field field : fields)
        field.getBody().accept(visitor);
    }

  }

  public static class Block extends Expression {

    private final List<Expression> exprs;

    public Block(List<Expression> exprs) {
      assert exprs.size() > 0;
      this.exprs = exprs;
    }

    @Override
    public void traverse(ExpressionVisitor visitor) {
      for (Expression expr : exprs)
        expr.accept(visitor);
    }

    public List<Expression> getExpressions() {
      return exprs;
    }

    @Override
    public String toString() {
      return Tree.toString(":", exprs);
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitBlock(this);
    }

  }

  public static class LocalDefinition extends Expression {

    private final String name;
    private final Expression value;
    private final Expression body;
    private final boolean isReference;
    private LocalSymbol symbol;

    public LocalDefinition(String name, Expression value, Expression body,
        boolean isReference) {
      this.name = name;
      this.value = value;
      this.body = body;
      this.isReference = isReference;
    }

    public void bind(LocalSymbol symbol) {
      assert this.symbol == null;
      this.symbol = symbol;
    }

    public boolean isReference() {
      return this.isReference;
    }

    public LocalSymbol getSymbol() {
      assert this.symbol != null;
      return this.symbol;
    }

    public String getName() {
      return name;
    }

    public Expression getValue() {
      return value;
    }

    public Expression getBody() {
      return body;
    }

    @Override
    public void traverse(ExpressionVisitor visitor) {
      value.accept(visitor);
      body.accept(visitor);
    }

    @Override
    public String toString() {
      return "(let (" + name + " " + value + ") " + body + ")";
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitLocalDefinition(this);
    }

  }

  private static String toString(String tag, List<?> elms) {
    StringBuilder buf = new StringBuilder();
    buf.append("(").append(tag);
    for (Object obj : elms)
      buf.append(" ").append(obj);
    return buf.append(")").toString();
  }

  private static String annotationsToString(List<Annotation> annots) {
    if (annots.isEmpty()) {
      return "";
    } else {
      StringBuilder result = new StringBuilder();
      result.append("(@");
      for (Annotation annot : annots) {
        List<RValue> args = annot.getArguments();
        result.append(" ");
        if (args.size() == 0) {
          result.append(annot.getTag());
        } else {
          result.append("(").append(annot.getTag());
          for (RValue arg : args) {
            result.append(" ").append(arg);
          }
          result.append(")");
        }
      }
      return result.append(") ").toString();
    }
  }

}
