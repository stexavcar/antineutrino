package org.javatrino.ast;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.List;

import org.neutrino.plankton.ClassIndex;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RProtocol;


public abstract class Expression {

  @Skip
  @Retention(RetentionPolicy.RUNTIME)
  private @interface Skip { }

  @Skip
  public enum Kind {
    CALL, BLOCK, ADD_INTRINSIC, CONST, DEF, GET_FIELD, SET_FIELD, GLOBAL,
    LOCAL, NEW, TAG_WITH_PROTOCOL
  }

  public final Kind kind;

  public Expression(Kind kind) {
    this.kind = kind;
  }

  public static class Call extends Expression {

    public static class Argument {

      public @Store Object tag;
      public @Store Expression value;

      public Argument(Object tag, Expression value) {
        this.tag = tag;
        this.value = value;
      }

      public Argument() { }

    }

    public @Store List<Argument> arguments;

    public Call(List<Argument> arguments) {
      this();
      this.arguments = arguments;
    }

    public Call() {
      super(Kind.CALL);
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

  }

  public static class NewObject extends Expression {

    public NewObject() {
      super(Kind.NEW);
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
      super(Kind.ADD_INTRINSIC);
    }

  }

  public static class TagWithProtocol extends Expression {

    public @Store Expression object;
    public @Store RProtocol protocol;

    public TagWithProtocol(Expression object, RProtocol protocol) {
      this();
      this.object = object;
      this.protocol = protocol;
    }

    public TagWithProtocol() {
      super(Kind.TAG_WITH_PROTOCOL);
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

  }

  public static void register(ClassIndex index) {
    index.add(Call.Argument.class);
    index.add(Symbol.class);
    for (Class<?> klass : Expression.class.getClasses()) {
      if (klass.getAnnotation(Skip.class) == null)
        index.add(klass);
    }
  }

}
