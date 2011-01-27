package org.neutrino.syntax;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.javatrino.ast.Symbol;
import org.neutrino.compiler.CompilerUniverse;
import org.neutrino.compiler.ResolverSymbol;
import org.neutrino.compiler.ResolverSymbol.LocalSymbol;
import org.neutrino.compiler.Source;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RFieldKey;
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

    public abstract String getName();

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

    @Override
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

    @Override
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

    @Override
    public String getName() {
      return null;
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

    @Override
    public String getName() {
      return null;
    }

    public String getMethodName() {
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

    public enum Type { LAMBDA };

    public abstract <T> T accept(ExpressionVisitor<T> visitor);

    public abstract void traverse(ExpressionVisitor<?> visitor);

    public RValue toValue() {
      assert false;
      return RNull.getInstance();
    }

    public Tree.Declaration getStaticValue(CompilerUniverse universe) {
      return null;
    }

    public Expression getAssignment(Expression value) {
      throw new UnsupportedOperationException();
    }

    public boolean is(Type type) {
      return false;
    }

  }

  public static class ExpressionVisitor<T> {

    public T visitExpression(Expression that) {
      that.traverse(this);
      return null;
    }

    public T visitLocalDefinition(LocalDefinition that) {
      return visitExpression(that);
    }

    public T visitBlock(Block that) {
      return visitExpression(that);
    }

    public T visitSingleton(Singleton that) {
      return visitExpression(that);
    }

    public T visitNumber(Number that) {
      return visitExpression(that);
    }

    public T visitIdentifier(Identifier that) {
      return visitExpression(that);
    }

    public T visitCall(Call that) {
      return visitExpression(that);
    }

    public T visitText(Text that) {
      return visitExpression(that);
    }

    public T visitNew(New that) {
      return visitExpression(that);
    }

    public T visitInternal(Internal that) {
      return visitExpression(that);
    }

    public T visitWith1Cc(With1Cc that) {
      return visitExpression(that);
    }

    public T visitAssignment(Assignment that) {
      return visitExpression(that);
    }

    public T visitCollection(Collection that) {
      return visitExpression(that);
    }

  }

  public static class Identifier extends Expression {

    private final String name;
    private ResolverSymbol resolverSymbol;
    private Symbol symbol;

    public Identifier(String name) {
      this.name = name;
    }

    @Override
    public Declaration getStaticValue(CompilerUniverse universe) {
      return resolverSymbol.getStaticValue(universe);
    }

    public String getName() {
      return name;
    }

    public void bind(ResolverSymbol resolverSymbol, Symbol symbol) {
      assert this.resolverSymbol == null;
      this.resolverSymbol = resolverSymbol;
      this.symbol = symbol;
    }

    public Symbol getSymbol() {
      return this.symbol;
    }

    public ResolverSymbol getResolverSymbol() {
      assert this.resolverSymbol != null;
      return this.resolverSymbol;
    }

    @Override
    public String toString() {
      return "$" + name;
    }

    @Override
    public Expression getAssignment(Expression value) {
      return new Assignment(this.name, value);
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) { }

    @Override
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitIdentifier(this);
    }

  }

  public static class Assignment extends Expression {

    private final String name;
    private final Tree.Expression value;
    private ResolverSymbol symbol;

    public Assignment(String name, Tree.Expression value) {
      this.name = name;
      this.value = value;
    }

    public ResolverSymbol getSymbol() {
      assert symbol != null;
      return symbol;
    }

    public String getName() {
      return this.name;
    }

    public Tree.Expression getValue() {
      return this.value;
    }

    public void bind(ResolverSymbol symbol) {
      assert this.symbol == null;
      this.symbol = symbol;
    }

    @Override
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitAssignment(this);
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) {
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
      return RInteger.get(value);
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) { }

    @Override
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitNumber(this);
    }

  }

  public static class Text extends Expression {

    private final String value;

    public Text(String value) {
      this.value = unescape(value);
    }

    private static final String unescape(String str) {
      StringBuilder buf = new StringBuilder();
      for (int i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (c == '\\') {
          i++;
          char n = str.charAt(i);
          switch (n) {
            case 'n':
              buf.append('\n');
              break;
            default:
              buf.append(n);
              break;
          }
        } else {
          buf.append(c);
        }
      }
      return buf.toString();
    }

    public String getValue() {
      return this.value;
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) { }

    @Override
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitText(this);
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
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitInternal(this);
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) {
      // ignore
    }

  }

  public static class Lambda {

    public static final String NAME = "()";

    public static Expression create(Source source, String methodName, List<Parameter> params,
        Expression body, String functionName) {
      New.Field call = new New.Field(params, methodName, body, false);
      return new New(source, Collections.singletonList(call),
          Collections.<String>emptyList(), functionName);
    }

    public static Expression create(Source source, List<Parameter> params,
        Expression body, String functionName) {
      return create(source, NAME, params, body, functionName);
    }

    public static Expression createCall(Expression fun, List<Expression> args) {
      List<Expression> allArgs = new ArrayList<Expression>();
      allArgs.add(fun);
      allArgs.addAll(args);
      return new Call(NAME, allArgs);
    }

  }

  public static class If {

    public static Expression create(Source source, Expression cond,
        Expression thenPart, Tree.Expression elsePart) {
      return new Call("if", Arrays.asList(
              new Identifier("Control"),
              cond,
              Lambda.create(source, Collections.<Parameter>emptyList(),
                  thenPart, "then"),
              Lambda.create(source ,Collections.<Parameter>emptyList(),
                  elsePart, "else")));
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
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitWith1Cc(this);
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) {
      callback.accept(visitor);
    }

    public static Expression create(Source source, String name, Expression body) {
      Parameter param = new Parameter(name, null, false);
      Expression lambda = Lambda.create(source, Collections.singletonList(param),
          body, "with_1cc");
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
    public void traverse(ExpressionVisitor<?> visitor) {
      for (Expression arg : args)
        arg.accept(visitor);
    }

    @Override
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitCall(this);
    }

    public List<Expression> getArguments() {
      return this.args;
    }

    public String getName() {
      return this.name;
    }

    @Override
    public Expression getAssignment(Expression value) {
      List<Expression> newArgs = new ArrayList<Expression>();
      newArgs.addAll(getArguments());
      newArgs.add(value);
      String newName = name + ":=";
      return new Call(newName, newArgs);
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
    public void traverse(ExpressionVisitor<?> visitor) { }

    @Override
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitSingleton(this);
    }

  }

  public static class Collection extends Expression {

    private final List<Expression> values;

    public Collection(List<Expression> values) {
      this.values = values;
    }

    public List<Expression> getValues() {
      return values;
    }

    @Override
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitCollection(this);
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) {
      for (Expression value : values)
        value.accept(visitor);
    }

  }

  public static class New extends Expression {

    public static class Field {

      private final List<Parameter> params;
      private final String name;
      private final Expression body;
      private final RFieldKey field;
      private int localCount = -1;

      public Field(List<Parameter> params, String name, Expression body,
          boolean hasEagerValue) {
        this.params = params;
        this.name = name;
        this.body = body;
        this.field = hasEagerValue ? new RFieldKey() : null;
      }

      public RFieldKey getField() {
        return this.field;
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
        return this.field != null;
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

    private final Source origin;
    private final List<Field> fields;
    private final List<String> protocolNames;
    private final List<Expression> protocols;
    private final String displayName;
    private RProtocol protocol;
    private List<ResolverSymbol> captures;
    private List<RFieldKey> captureFields;

    public New(Source origin, List<Field> fields, List<String> protocolNames,
        String displayName) {
      this.origin = origin;
      this.fields = fields;
      this.protocolNames = protocolNames;
      this.protocols = new ArrayList<Expression>();
      for (String name : protocolNames)
        protocols.add(new Identifier(name));
      this.displayName = displayName;
    }

    public Source getOrigin() {
      return origin;
    }

    private boolean isLambda() {
      for (Field field : fields) {
        if (field.getName().equals(Lambda.NAME))
          return true;
      }
      return false;
    }

    @Override
    public boolean is(Type type) {
      return (type == Type.LAMBDA) && isLambda();
    }

    public List<Expression> getProtocols() {
      return this.protocols;
    }

    public List<String> getProtocolNames() {
      return protocolNames;
    }

    public String getDisplayName() {
      return this.displayName;
    }

    public void setCaptures(List<ResolverSymbol> captures, List<RFieldKey> fields) {
      assert this.captures == null;
      this.captures = captures;
      this.captureFields = fields;
    }

    public List<ResolverSymbol> getCaptures() {
      assert this.captures != null;
      return this.captures;
    }

    public List<RFieldKey> getCaptureFields() {
      return this.captureFields;
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
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitNew(this);
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) {
      for (Field field : fields)
        field.getBody().accept(visitor);
      for (Expression protocol : this.protocols)
        protocol.accept(visitor);
    }

  }

  public static class Block extends Expression {

    private final List<Expression> exprs;

    public Block(List<Expression> exprs) {
      assert exprs.size() > 0;
      this.exprs = exprs;
    }

    @Override
    public void traverse(ExpressionVisitor<?> visitor) {
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
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitBlock(this);
    }

  }

  public static class LocalDefinition extends Expression {

    private final List<Annotation> annots;
    private final String name;
    private final Expression value;
    private final Expression body;
    private final boolean isReference;
    private final Symbol symbol;
    private LocalSymbol resolverSymbol;

    public LocalDefinition(List<Annotation> annots, String name,
        Expression value, Expression body, boolean isReference) {
      this.annots = annots;
      this.name = name;
      this.value = value;
      this.body = body;
      this.isReference = isReference;
      this.symbol = new Symbol(Symbol.kLocalSymbol);
    }

    public void bind(LocalSymbol resolverSymbol) {
      assert this.resolverSymbol == null;
      this.resolverSymbol = resolverSymbol;
    }

    public boolean isReference() {
      return this.isReference;
    }

    public LocalSymbol getResolverSymbol() {
      assert this.resolverSymbol != null;
      return this.resolverSymbol;
    }

    public Symbol getSymbol() {
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
    public void traverse(ExpressionVisitor<?> visitor) {
      value.accept(visitor);
      body.accept(visitor);
    }

    @Override
    public String toString() {
      return "(let (" + name + " " + value + ") " + body + ")";
    }

    @Override
    public <T> T accept(ExpressionVisitor<T> visitor) {
      return visitor.visitLocalDefinition(this);
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
