package org.javatrino.ast;

import java.util.List;

import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RProtocol;
import org.neutrino.runtime.RValue;


public abstract class Expression {

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

      private final RValue tag;
      private final Expression value;

      public Argument(RValue tag, Expression value) {
        this.tag = tag;
        this.value = value;
      }

    }

    private final List<Argument> arguments;

    public Call(List<Argument> arguments) {
      super(Kind.CALL);
      this.arguments = arguments;
    }

  }

  public static class Block extends Expression {

    private final List<Expression> values;

    public Block(List<Expression> values) {
      super(Kind.BLOCK);
      this.values = values;
    }

  }

  public static class Definition extends Expression {

    private final Symbol symbol;
    private final Expression value;
    private final Expression body;

    public Definition(Symbol symbol, Expression value, Expression body) {
      super(Kind.DEF);
      this.symbol = symbol;
      this.value = value;
      this.body = body;
    }

  }

  public static class Local extends Expression {

    private final Symbol symbol;

    public Local(Symbol symbol) {
      super(Kind.LOCAL);
      this.symbol = symbol;
    }

  }

  public static class Global extends Expression {

    private final RValue name;

    public Global(RValue name) {
      super(Kind.GLOBAL);
      this.name = name;
    }

  }

  public static class Constant extends Expression {

    private final RValue value;

    public Constant(RValue value) {
      super(Kind.CONST);
      this.value = value;
    }

  }

  public static class NewObject extends Expression {

    public NewObject() {
      super(Kind.NEW);
    }

  }

  public static class AddIntrinsics extends Expression {

    private final Expression object;
    private final List<Method> methods;

    public AddIntrinsics(Expression object, List<Method> methods) {
      super(Kind.ADD_INTRINSIC);
      this.object = object;
      this.methods = methods;
    }

  }

  public static class TagWithProtocol extends Expression {

    private final Expression object;
    private final RProtocol protocol;

    public TagWithProtocol(Expression object, RProtocol protocol) {
      super(Kind.TAG_WITH_PROTOCOL);
      this.object = object;
      this.protocol = protocol;
    }

  }

  public static class SetField extends Expression {

    private final Expression object;
    private final RFieldKey field;
    private final Expression value;

    public SetField(Expression object, RFieldKey field, Expression value) {
      super(Kind.SET_FIELD);
      this.object = object;
      this.field = field;
      this.value = value;
    }

  }

  public static class GetField extends Expression {

    private final Expression object;
    private final RFieldKey field;

    public GetField(Expression object, RFieldKey field) {
      super(Kind.GET_FIELD);
      this.object = object;
      this.field = field;
    }

  }

}
