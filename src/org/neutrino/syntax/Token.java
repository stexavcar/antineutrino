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
    OPERATOR(null, false, true),
    ERROR,
    LPAREN("("),
    RPAREN(")"),
    LBRACE("{"),
    RBRACE("}"),
    SEMI(";"),
    COMMA(","),
    COLON(":"),
    COLON_EQ(":="),
    ARROW("->", false, true),
    AT("@", false, true),
    DOT(".", false, true),
    DEF("def", true, false),
    FN("fn", true, false),
    NEW("new", true, false),
    NULL("null", true, false),
    TRUE("true", true, false),
    FALSE("false", true, false),
    PROTOCOL("protocol", true, false);

    private final String name;
    private final boolean isKeyword;
    private final boolean isOperator;

    private Type(String name, boolean isKeyword, boolean isOperator) {
      this.name = name;
      this.isKeyword = isKeyword;
      this.isOperator = isOperator;
    }

    private Type(String name) {
      this(name, false, false);
    }

    private Type() {
      this(null, false, false);
    }

    public String getName() {
      return this.name;
    }

    public boolean isOperator() {
      return this.isOperator;
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

    public static final Map<String, Type> RESERVED_OPERATOR_MAP = new HashMap<String, Type>() {{
      for (Type type : Type.values()) {
        if (type.getName() != null && type.isOperator)
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
