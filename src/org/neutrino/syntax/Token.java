package org.neutrino.syntax;

import java.util.HashMap;
import java.util.Map;

/**
 * A single token in the input source code.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Token {

  public enum Type {

    IDENT,
    NUMBER,
    ERROR,
    LPAREN("("),
    RPAREN(")"),
    LBRACE("{"),
    RBRACE("}"),
    SEMI(";"),
    COMMA(","),
    DASH("-"),
    ARROW("->"),
    GT(">"),
    COLON(":"),
    AT("@"),
    COLON_EQ(":="),
    DEF("def", true),
    FN("fn", true),
    NEW("new", true),
    NULL("null", true);

    private final String name;
    private final boolean isKeyword;

    private Type(String name, boolean isKeyword) {
      this.name = name;
      this.isKeyword = isKeyword;
    }

    private Type(String name) {
      this(name, false);
    }

    private Type() {
      this(null, false);
    }

    public String getName() {
      return this.name;
    }

    @Override
    public String toString() {
      return (this.name == null) ? super.toString() : this.name;
    }

    public static final Map<String, Type> KEYWORD_MAP = new HashMap<String, Type>() {{
      for (Type type : Type.values()) {
        if (type.isKeyword)
          put(type.getName(), type);
      }
    }};

  }

  private final Type type;
  private final String value;
  private final int start;
  private final int end;

  public Token(Type type, String value, int start, int end) {
    this.type = type;
    this.value = value;
    this.start = start;
    this.end = end;
  }

  public Type getType() {
    return this.type;
  }

  public String getValue() {
    return this.value;
  }

  public int getStart() {
    return this.start;
  }

  public int getEnd() {
    return this.end;
  }

  public String toString() {
    if (getType().getName() != null) {
      return this.type.toString();
    } else {
      return this.type + ":" + this.value;
    }
  }

  @Override
  public int hashCode() {
    if (this.value == null) {
      return this.type.hashCode();
    } else {
      return this.type.hashCode() ^ this.value.hashCode();
    }
  }

  @Override
  public boolean equals(Object obj) {
    if (this == obj)
      return true;
    if (!(obj instanceof Token))
      return false;
    Token that = (Token) obj;
    return that.type.equals(this.type) && that.value.equals(this.value);
  }

}
