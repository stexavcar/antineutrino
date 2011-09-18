package org.javatrino.ast;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.Arrays;
import java.util.List;

import org.neutrino.pib.Module;
import org.neutrino.plankton.ClassIndex;
import org.neutrino.plankton.Store;
import org.neutrino.runtime.RFieldKey;
import org.neutrino.runtime.RValue;


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

    public void visitNewObjectArray(NewObjectArray that) {
      visitExpression(that);
    }

    public void visitWithEscape(WithEscape that) {
      visitExpression(that);
    }

    public void visitInternal(Internal that) {
      visitExpression(that);
    }

    public void visitNewDictionary(NewDictionary that) {
      visitExpression(that);
    }

  }

  public abstract void accept(Visitor visitor);

  public abstract void traverse(Visitor visitor);

  /**
   * Returns the compile-time known static value of this expression.
   */
  public RValue getStaticValue(Module module) {
    return null;
  }

  public static class Call extends Expression {

    public static class Argument {

      public @Store RValue tag;
      public @Store Expression value;

      private Argument(RValue tag, Expression value) {
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

    private Call(List<Argument> arguments) {
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

    private Block(List<Expression> values) {
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

    private Definition(Symbol symbol, Expression value, Expression body) {
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

    private Local(Symbol symbol) {
      assert symbol != null;
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

    public @Store RValue name;

    private Global(RValue name) {
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
    public RValue getStaticValue(Module module) {
      RValue result = module.getGlobal(name);
      if (result == null) {
        module.getGlobal(name);
        throw new AssertionError("Couldn't find " + name + " from "
            + module.wrapper);
      }
      return result;
    }

    @Override
    public String toString() {
      return "$" + name;
    }

  }

  public static class Constant extends Expression {

    public @Store RValue value;

    private Constant(RValue value) {
      this.value = value;
    }

    public Constant() { }

    @Override
    public String toString() {
      return String.valueOf(value);
    }

    @Override
    public RValue getStaticValue(Module module) {
      return value;
    }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitConstant(this);
    }

    @Override
    public void traverse(Visitor visitor) {
    }

  }

  public static class Internal extends Expression {

    public @Store Object name;
    public @Store int argc;

    private Internal(Object name, int argc) {
      this.name = name;
      this.argc = argc;
    }

    public Internal() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitInternal(this);
    }

    @Override
    public void traverse(Visitor visitor) {
    }

  }

  public static class NewObject extends Expression {

    public NewObject() { }

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

  public static class NewObjectArray extends Expression {

    public @Store List<Expression> values;

    private NewObjectArray(List<Expression> values) {
      this.values = values;
    }

    public NewObjectArray() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitNewObjectArray(this);
    }

    @Override
    public void traverse(Visitor visitor) {
      for (Expression value : values)
        value.accept(visitor);
    }

  }

  public static class NewDictionary extends Expression {

    public static class Entry {

      public @Store Expression key;
      public @Store Expression value;

      public Entry() { }

      public Entry(Expression key, Expression value) {
        this.key = key;
        this.value = value;
      }

    }

    public @Store List<Entry> entries;

    public NewDictionary() { }

    public NewDictionary(List<Entry> entries) {
      this.entries = entries;
    }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitNewDictionary(this);
    }

    public void traverse(Visitor visitor) {
      for (Entry entry : entries) {
        entry.key.accept(visitor);
        entry.value.accept(visitor);
      }
    }

  }

  public static class WithEscape extends Expression {

    public @Store Expression body;
    public @Store Symbol symbol;

    private WithEscape(Expression body, Symbol symbol) {
      this.body = body;
      this.symbol = symbol;
    }

    public WithEscape() { }

    @Override
    public void accept(Visitor visitor) {
      visitor.visitWithEscape(this);
    }

    @Override
    public void traverse(Visitor visitor) {
      body.accept(visitor);
    }

  }

  public static class AddIntrinsics extends Expression {

    public @Store Expression object;
    public @Store List<Method> methods;

    private AddIntrinsics(Expression object, List<Method> methods) {
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

    private TagWithProtocol(Expression object, List<Expression> protocols) {
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

    private SetField(Expression object, RFieldKey field, Expression value) {
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

    @Override
    public String toString() {
      return "setField(" + object + ", " + field + ", " + value + ")";
    }

  }

  public static class GetField extends Expression {

    public @Store Expression object;
    public @Store RFieldKey field;

    private GetField(Expression object, RFieldKey field) {
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
    index.add(NewDictionary.Entry.class);
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

  @Skip
  public static class StaticFactory {

    public static NewObject eNewObject() {
      return new NewObject();
    }

    public static Local eLocal(Symbol symbol) {
      return new Local(symbol);
    }

    public static GetField eGetField(Expression obj, RFieldKey field) {
      return new GetField(obj, field);
    }

    public static NewObjectArray eNewObjectArray(List<Expression> exprs) {
      return new NewObjectArray(exprs);
    }

    public static NewDictionary eNewDictionary(List<NewDictionary.Entry> exprs) {
      return new NewDictionary(exprs);
    }

    public static SetField eSetField(Expression obj, RFieldKey field, Expression value) {
      return new SetField(obj, field, value);
    }

    public static AddIntrinsics eAddIntrinsics(Expression obj, List<Method> methods) {
      return new AddIntrinsics(obj, methods);
    }

    public static Block eBlock(List<Expression> values) {
      return new Block(values);
    }

    public static Call.Argument eArgument(RValue tag, Expression value) {
      return new Call.Argument(tag, value);
    }

    public static Call eCall(List<Call.Argument> args) {
      return new Call(args);
    }

    public static Call eCall(Call.Argument... args) {
      return new Call(Arrays.asList(args));
    }

    public static Constant eConstant(RValue value) {
      return new Constant(value);
    }

    public static Internal eInternal(Object name, int argc) {
      return new Internal(name, argc);
    }

    public static Definition eDefinition(Symbol symbol, Expression value, Expression body) {
      return new Definition(symbol, value, body);
    }

    public static Global eGlobal(RValue name) {
      return new Global(name);
    }

    public static TagWithProtocol eTagWithProtocol(Expression obj, List<Expression> protos) {
      return new TagWithProtocol(obj, protos);
    }

    public static WithEscape eWithEscape(Symbol symbol, Expression body) {
      return new WithEscape(body, symbol);
    }

  }

}
