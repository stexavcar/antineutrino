package org.neutrino.syntax;

import java.util.ArrayList;
import java.util.List;

/**
 * A simple scanner that turns a flat string into a list of neutrino
 * source tokens.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Scanner {

  private int cursor = 0;
  private final String source;
  private int startOffset = 0;
  private int endOffset = 0;

  private Scanner(String source) {
    this.source = source;
    skipWhiteSpace();
  }

  private static boolean isWhitespace(char c) {
    return Character.isWhitespace(c);
  }

  private static boolean isIdentifierStart(char c) {
    return Character.isJavaIdentifierStart(c);
  }

  private static boolean isIdentifierPart(char c) {
    return Character.isJavaIdentifierPart(c);
  }

  private static boolean isDigit(char c) {
    return Character.isDigit(c);
  }

  private static boolean isOperator(char c) {
    switch (c) {
    case '+': case '-': case '.': case '@': case '%': case '*':
    case '>': case '<': case '!': case '?': case '=':
      return true;
    default:
      return false;
    }
  }

  /**
   * Skip over zero or more characters for whom {@link #isWhitespace(char)}
   * returns true.
   */
  private void skipWhiteSpace() {
    while (hasMore() && isWhitespace(getCurrent()))
      advance();
  }

  /**
   * Are there more unread characters?
   */
  private boolean hasMore() {
    return cursor < source.length();
  }

  /**
   * Returns the character at the current cursor.
   */
  private char getCurrent() {
    return source.charAt(cursor);
  }

  /**
   * Returns the position of the current character in the source string.
   */
  private int getCursor() {
    return cursor;
  }

  private String getSubstring(int start, int end) {
    return source.substring(start, end);
  }

  /**
   * Move the cursor one character forward.
   */
  private void advance() {
    cursor++;
  }

  private void fail() throws SyntaxError {
    String value = new String(new char[] { getCurrent() });
    int start = cursor;
    int end = cursor + 1;
    Token token = new Token(Token.Type.ERROR, value, start, end);
    throw new SyntaxError(token);
  }

  /**
   * Scan over and return the next token in the input.
   */
  private Token.Type scanNext() throws SyntaxError {
    char c = getCurrent();
    switch (c) {
    case '(':
      advance();
      return Token.Type.LPAREN;
    case ')':
      advance();
      return Token.Type.RPAREN;
    case '{':
      advance();
      return Token.Type.LBRACE;
    case '}':
      advance();
      return Token.Type.RBRACE;
    case ';':
      advance();
      return Token.Type.SEMI;
    case ',':
      advance();
      return Token.Type.COMMA;
    case ':':
      advance();
      if (hasMore()) {
        switch (getCurrent()) {
        case '=':
          advance();
          return Token.Type.COLON_EQ;
        case ':':
          advance();
          return Token.Type.COOLON;
        }
      }
      return Token.Type.COLON;
    case '"':
      return scanString();
    }
    if (isIdentifierStart(c)) {
      return scanIdentifier();
    } else if (isDigit(c)) {
      return scanNumber();
    } else if (isOperator(c)) {
      return scanOperator();
    } else {
      fail();
      return null;
    }
  }

  /**
   * Scan over and return an identifier.
   */
  private Token.Type scanIdentifier() {
    assert isIdentifierStart(getCurrent());
    int start = getCursor();
    while (hasMore() && isIdentifierPart(getCurrent()))
      advance();
    int end = getCursor();
    String value = getSubstring(start, end);
    Token.Type keyword = Token.Type.KEYWORD_MAP.get(value);
    return keyword == null ? Token.Type.IDENT : keyword;
  }

  private Token.Type scanString() {
    assert getCurrent() == '"';
    startOffset = 1;
    advance();
    while (hasMore() && getCurrent() != '"')
      advance();
    if (hasMore()) {
      endOffset = -1;
      advance();
    }
    return Token.Type.STRING;
  }

  private Token.Type scanOperator() {
    assert isOperator(getCurrent());
    int start = getCursor();
    while (hasMore() && isOperator(getCurrent()))
      advance();
    int end = getCursor();
    String value = getSubstring(start, end);
    Token.Type reserved = Token.Type.RESERVED_OPERATOR_MAP.get(value);
    return reserved == null ? Token.Type.OPERATOR : reserved;
  }

  private Token.Type scanNumber() {
    assert isDigit(getCurrent());
    while (hasMore() && isDigit(getCurrent()))
      advance();
    return Token.Type.NUMBER;
  }

  private List<Token> scan() throws SyntaxError {
    List<Token> tokens = new ArrayList<Token>();
    while (hasMore()) {
      startOffset = endOffset = 0;
      int start = getCursor();
      Token.Type nextType = scanNext();
      if (nextType == null)
        return null;
      int end = getCursor() + endOffset;
      String text = getSubstring(start + startOffset, end);
      tokens.add(new Token(nextType, text, start, end));
      skipWhiteSpace();
    }
    return tokens;
  }

  public static List<Token> tokenize(String source) throws SyntaxError {
    return new Scanner(source).scan();
  }

}
