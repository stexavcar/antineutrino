package org.neutrino.syntax;

import java.util.List;

/**
 * Neutrino syntax trees.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Tree {

  public static class Unit extends Tree {

    private final List<Definition> defs;

    public Unit(List<Definition> defs) {
      this.defs = defs;
    }

    public List<Definition> getDefinitions() {
      return this.defs;
    }

    @Override
    public String toString() {
      return toString("unit", defs);
    }

  }

  public static class Definition extends Tree {

    private final List<String> annots;
    private final String name;
    private final Expression value;

    public Definition(List<String> annots, String name, Expression value) {
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

    public List<String> getAnnotations() {
      return this.annots;
    }

    @Override
    public String toString() {
      String an = annots.isEmpty() ? "" : (toString("@", annots) + " ");
      return "(def " + an + name + " " + value + ")";
    }

  }

  public static abstract class Expression extends Tree {

    public abstract void accept(ExpressionVisitor visitor);

  }

  public static interface ExpressionVisitor {

    public void visitLocalDefinition(LocalDefinition that);

    public void visitBlock(Block that);

    public void visitSingleton(Singleton that);

    public void visitLambda(Lambda that);

    public void visitNumber(Number that);

    public void visitIdentifier(Identifier that);

  }

  public static class Identifier extends Expression {

    private final String name;

    public Identifier(String name) {
      this.name = name;
    }

    @Override
    public String toString() {
      return "$" + name;
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitIdentifier(this);
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
    public void accept(ExpressionVisitor visitor) {
      visitor.visitNumber(this);
    }

  }

  public static class Lambda extends Expression {

    private final List<String> params;
    private final Expression body;

    public Lambda(List<String> params, Expression body) {
      this.params = params;
      this.body = body;
    }

    @Override
    public String toString() {
      return "(fn " + toString(params) + " " + body + ")";
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitLambda(this);
    }

  }

  public static class Singleton extends Expression {

    public enum Type { NULL, TRUE, FALSE };

    private final Type type;

    public Singleton(Type type) {
      this.type = type;
    }

    @Override
    public String toString() {
      return type.name().toLowerCase();
    }

    @Override
    public void accept(ExpressionVisitor visitor) {
      visitor.visitSingleton(this);
    }

  }

  public static class Block extends Expression {

    private final List<Expression> exprs;

    public Block(List<Expression> exprs) {
      this.exprs = exprs;
    }

    @Override
    public String toString() {
      return toString(":", exprs);
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

    public LocalDefinition(String name, Expression value, Expression body) {
      this.name = name;
      this.value = value;
      this.body = body;
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

  private static String toString(List<?> elms) {
    StringBuilder buf = new StringBuilder();
    buf.append("(");
    boolean first = true;
    for (Object obj : elms) {
      if (first) {
        first = false;
      } else {
        buf.append(" ");
      }
      buf.append(obj);
    }
    return buf.append(")").toString();
  }

}
