package org.javatrino.ast;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.List;

import org.neutrino.plankton.ClassIndex;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.RFieldKey;


public abstract class Expression {

  @Skip
  @Retention(RetentionPolicy.RUNTIME)
  private @interface Skip { }

  @Skip
  public static abstract class Visitor<T> {

    public T visitExpression(Expression expr) {
      expr.traverse(this);
      return null;
    }

    public T visitCall(Call that) {
      return visitExpression(that);
    }

    public T visitBlock(Block that) {
      return visitExpression(that);
    }

    public T visitDefinition(Definition that) {
      return visitExpression(that);
    }

    public T visitLocal(Local that) {
      return visitExpression(that);
    }

    public T visitGlobal(Global that) {
      return visitExpression(that);
    }

    public T visitConstant(Constant that) {
      return visitExpression(that);
    }

    public T visitNewObject(NewObject that) {
      return visitExpression(that);
    }

    public T visitAddIntrinsics(AddIntrinsics that) {
      return visitExpression(that);
    }

    public T visitTagWithProtocol(TagWithProtocol that) {
      return visitExpression(that);
    }

    public T visitSetField(SetField that) {
      return visitExpression(that);
    }

    public T visitGetField(GetField that) {
      return visitExpression(that);
    }

  }

  @Skip
  public enum Kind {
    CALL, BLOCK, ADD_INTRINSICS, CONST, DEF, GET_FIELD, SET_FIELD, GLOBAL,
    LOCAL, NEW, TAG_WITH_PROTOCOL
  }

  public final Kind kind;

  public Expression(Kind kind) {
    this.kind = kind;
  }

  public abstract <T> T accept(Visitor<T> visitor);

  public abstract void traverse(Visitor<?> visitor);

  public static class Call extends Expression {

    public static class Argument {

      public @Store Object tag;
      public @Store Expression value;

      public Argument(Object tag, Expression value) {
        this.tag = tag;
        this.value = value;
      }

      public Argument() { }

      @Override
      public String toString() {
        return tag + ": " + value;
      }

    }

    public @Store List<Argument> arguments;

    public Call(List<Argument> arguments) {
      this();
      this.arguments = arguments;
    }

    public Call() {
      super(Kind.CALL);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitCall(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
      for (Argument arg : arguments)
        arg.value.accept(visitor);
    }

    @Override
    public String toString() {
      return "call" + arguments.toString();
    }

  }

  public static class Block extends Expression {

    public @Store List<Expression> values;

    public Block(List<Expression> values) {
      this();
      this.values = values;
    }

    public Block() {
      super(Kind.BLOCK);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitBlock(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
      for (Expression value : values)
        value.accept(visitor);
    }

  }

  public static class Definition extends Expression {

    public @Store Symbol symbol;
    public @Store Expression value;
    public @Store Expression body;

    public Definition(Symbol symbol, Expression value, Expression body) {
      this();
      this.symbol = symbol;
      this.value = value;
      this.body = body;
    }

    public Definition() {
      super(Kind.DEF);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitDefinition(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
      value.accept(visitor);
      body.accept(visitor);
    }

    @Override
    public String toString() {
      return "def " + symbol + " := " + value + " in " + body;
    }

  }

  public static class Local extends Expression {

    public @Store Symbol symbol;

    public Local(Symbol symbol) {
      this();
      this.symbol = symbol;
    }

    public Local() {
      super(Kind.LOCAL);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitLocal(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
    }

    @Override
    public String toString() {
      return symbol.toString();
    }

  }

  public static class Global extends Expression {

    public @Store Object name;

    public Global(Object name) {
      this();
      this.name = name;
    }

    public Global() {
      super(Kind.GLOBAL);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitGlobal(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
    }

    @Override
    public String toString() {
      return "$" + name;
    }

  }

  public static class Constant extends Expression {

    public @Store Object value;

    public Constant(Object value) {
      this();
      this.value = value;
    }

    public Constant() {
      super(Kind.CONST);
    }

    @Override
    public String toString() {
      return String.valueOf(value);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitConstant(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
    }

  }

  public static class NewObject extends Expression {

    public NewObject() {
      super(Kind.NEW);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitNewObject(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
    }

  }

  public static class AddIntrinsics extends Expression {

    public @Store Expression object;
    public @Store List<Method> methods;

    public AddIntrinsics(Expression object, List<Method> methods) {
      this();
      this.object = object;
      this.methods = methods;
    }

    public AddIntrinsics() {
      super(Kind.ADD_INTRINSICS);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitAddIntrinsics(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
      object.accept(visitor);
    }

  }

  public static class TagWithProtocol extends Expression {

    public @Store Expression object;
    public @Store List<Expression> protocols;

    public TagWithProtocol(Expression object, List<Expression> protocols) {
      this();
      this.object = object;
      this.protocols = protocols;
    }

    public TagWithProtocol() {
      super(Kind.TAG_WITH_PROTOCOL);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitTagWithProtocol(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
      object.accept(visitor);
      for (Expression protocol : protocols)
        protocol.accept(visitor);
    }

  }

  public static class SetField extends Expression {

    public @Store Expression object;
    public @Store RFieldKey field;
    public @Store Expression value;

    public SetField(Expression object, RFieldKey field, Expression value) {
      this();
      this.object = object;
      this.field = field;
      this.value = value;
    }

    public SetField() {
      super(Kind.SET_FIELD);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitSetField(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
      object.accept(visitor);
      value.accept(visitor);
    }

  }

  public static class GetField extends Expression {

    public @Store Expression object;
    public @Store RFieldKey field;

    public GetField(Expression object, RFieldKey field) {
      this();
      this.object = object;
      this.field = field;
    }

    public GetField() {
      super(Kind.GET_FIELD);
    }

    @Override
    public <T> T accept(Visitor<T> visitor) {
      return visitor.visitGetField(this);
    }

    @Override
    public void traverse(Visitor<?> visitor) {
      object.accept(visitor);
    }

  }

  public static void register(ClassIndex index) {
    index.add(Call.Argument.class);
    index.add(Symbol.class);
    for (Class<?> klass : Expression.class.getClasses()) {
      if (klass.getAnnotation(Skip.class) == null)
        index.add(klass);
    }
    index.add(Method.class);
    index.add(Pattern.class);
    index.add(Test.Any.class);
    index.add(Test.Eq.class);
    index.add(Test.Is.class);
  }

}
