package org.neutrino.syntax;

import java.util.HashMap;
import java.util.Map;

/**
 * A single token in the input source code.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Token {

  private enum Category {
    KEYWORD(true, false, false),
    OPERATOR(false, true, false),
    RESERVED_OPERATOR(false, false, true),
    NONE(false, false, false);
    private boolean isKeyword;
    private boolean isOperator;
    private boolean reserveOperator;
    private Category(boolean isKeyword, boolean isOperator, boolean reserveOperator) {
      this.isKeyword = isKeyword;
      this.isOperator = isOperator;
      this.reserveOperator = reserveOperator;
    }
  }

  public enum Type {

    IDENT,
    NUMBER,
    STRING,
    INTERPOL_START,
    INTERPOL_PART,
    INTERPOL_END,
    OPERATOR(null, Category.OPERATOR),
    ERROR,
    LPAREN("("),
    RPAREN(")"),
    LBRACE("{"),
    RBRACE("}"),
    LBRACK("["),
    RBRACK("]"),
    SEMI(";"),
    COMMA(","),
    HASH("#"),
    COLON(":"),
    COOLON("::"),
    COLON_EQ(":="),
    ARROW("->", Category.RESERVED_OPERATOR),
    FAT_ARROW("=>", Category.RESERVED_OPERATOR),
    AT("@", Category.RESERVED_OPERATOR),
    DOT(".", Category.RESERVED_OPERATOR),
    DEF("def", Category.KEYWORD),
    FN("fn", Category.KEYWORD),
    NEW("new", Category.KEYWORD),
    NULL("null", Category.KEYWORD),
    TRUE("true", Category.KEYWORD),
    FALSE("false", Category.KEYWORD),
    IF("if", Category.KEYWORD),
    FOR("for", Category.KEYWORD),
    THEN("then", Category.KEYWORD),
    ELSE("else", Category.KEYWORD),
    AND("and", Category.KEYWORD),
    OR("or", Category.KEYWORD),
    NOT("not", Category.KEYWORD),
    REF("ref", Category.KEYWORD),
    IS("is", Category.KEYWORD),
    INTERNAL("internal", Category.KEYWORD),
    WITH_ESCAPE("with_escape", Category.KEYWORD),
    PROTOCOL("protocol", Category.KEYWORD);

    private final String name;
    private final Category category;

    private Type(String name, Category category) {
      this.name = name;
      this.category = category;
    }

    private Type(String name) {
      this(name, Category.NONE);
    }

    private Type() {
      this(null, Category.NONE);
    }

    public String getName() {
      return this.name;
    }

    public boolean isOperator() {
      return this.category.isOperator;
    }

    @Override
    public String toString() {
      return (this.name == null) ? super.toString() : this.name;
    }

    @SuppressWarnings("serial")
    public static final Map<String, Type> KEYWORD_MAP = new HashMap<String, Type>() {{
      for (Type type : Type.values()) {
        if (type.category.isKeyword)
          put(type.getName(), type);
      }
    }};

    @SuppressWarnings("serial")
    public static final Map<String, Type> RESERVED_OPERATOR_MAP = new HashMap<String, Type>() {{
      for (Type type : Type.values()) {
        if (type.getName() != null && type.category.reserveOperator)
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

  @Override
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
