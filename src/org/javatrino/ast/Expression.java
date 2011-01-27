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
  public static abstract class Visitor {

    public void visitExpression(Expression expr) {
      expr.traverse(this);
    }

    public void visitCall(Call that) {
      visitExpression(that);
    }

    public void visitBlock(Block that) {
      visitExpression(that);
    }

    public void visitDefinition(Definition that) {
      visitExpression(that);
    }

    public void visitLocal(Local that) {
      visitExpression(that);
    }

    public void visitGlobal(Global that) {
      visitExpression(that);
    }

    public void visitConstant(Constant that) {
      visitExpression(that);
    }

    public void visitNewObject(NewObject that) {
      visitExpression(that);
    }

    public void visitAddIntrinsics(AddIntrinsics that) {
      visitExpression(that);
    }

    public void visitTagWithProtocol(TagWithProtocol that) {
      visitExpression(that);
    }

    public void visitSetField(SetField that) {
      visitExpression(that);
    }

    public void visitGetField(GetField that) {
      visitExpression(that);
    }

  }

  public abstract void accept(Visitor visitor);

  public abstract void traverse(Visitor visitor);

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
      this.arguments = arguments;
    }

    public Call() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitCall(this);
    }

    @Override
    public void traverse(Visitor visitor) {
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
      this.values = values;
    }

    public Block() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitBlock(this);
    }

    @Override
    public void traverse(Visitor visitor) {
      for (Expression value : values)
        value.accept(visitor);
    }

    @Override
    public String toString() {
      return "{" + values + "}";
    }

  }

  public static class Definition extends Expression {

    public @Store Symbol symbol;
    public @Store Expression value;
    public @Store Expression body;

    public Definition(Symbol symbol, Expression value, Expression body) {
      this.symbol = symbol;
      this.value = value;
      this.body = body;
    }

    public Definition() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitDefinition(this);
    }

    @Override
    public void traverse(Visitor visitor) {
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
      this.symbol = symbol;
    }

    public Local() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitLocal(this);
    }

    @Override
    public void traverse(Visitor visitor) {
    }

    @Override
    public String toString() {
      return symbol.toString();
    }

  }

  public static class Global extends Expression {

    public @Store Object name;

    public Global(Object name) {
      this.name = name;
    }

    public Global() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitGlobal(this);
    }

    @Override
    public void traverse(Visitor visitor) {
    }

    @Override
    public String toString() {
      return "$" + name;
    }

  }

  public static class Constant extends Expression {

    public @Store Object value;

    public Constant(Object value) {
      this.value = value;
    }

    public Constant() { }

    @Override
    public String toString() {
      return String.valueOf(value);
    }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitConstant(this);
    }

    @Override
    public void traverse(Visitor visitor) {
    }

  }

  public static class NewObject extends Expression {

    @Override
    public void accept(Visitor visitor) {
      visitor.visitNewObject(this);
    }

    @Override
    public void traverse(Visitor visitor) {
    }

    @Override
    public String toString() {
      return "new()";
    }

  }

  public static class AddIntrinsics extends Expression {

    public @Store Expression object;
    public @Store List<Method> methods;

    public AddIntrinsics(Expression object, List<Method> methods) {
      this.object = object;
      this.methods = methods;
    }

    public AddIntrinsics() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitAddIntrinsics(this);
    }

    @Override
    public void traverse(Visitor visitor) {
      object.accept(visitor);
    }

    @Override
    public String toString() {
      return "add_intrinsics(" + object + ", " + methods + ")";
    }

  }

  public static class TagWithProtocol extends Expression {

    public @Store Expression object;
    public @Store List<Expression> protocols;

    public TagWithProtocol(Expression object, List<Expression> protocols) {
      this.object = object;
      this.protocols = protocols;
    }

    public TagWithProtocol() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitTagWithProtocol(this);
    }

    @Override
    public void traverse(Visitor visitor) {
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
      this.object = object;
      this.field = field;
      this.value = value;
    }

    public SetField() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitSetField(this);
    }

    @Override
    public void traverse(Visitor visitor) {
      object.accept(visitor);
      value.accept(visitor);
    }

  }

  public static class GetField extends Expression {

    public @Store Expression object;
    public @Store RFieldKey field;

    public GetField(Expression object, RFieldKey field) {
      this.object = object;
      this.field = field;
    }

    public GetField() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitGetField(this);
    }

    @Override
    public void traverse(Visitor visitor) {
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
