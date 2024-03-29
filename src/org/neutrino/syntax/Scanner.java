package org.neutrino.syntax;

import java.util.ArrayList;
import java.util.List;
import java.util.Stack;

import org.neutrino.compiler.Source;

/**
 * A simple scanner that turns a flat string into a list of neutrino
 * source tokens.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Scanner {

  enum ContextType { TOP, DELIM, INTERPOL }

  private final Source origin;
  private int cursor = 0;
  private final String source;
  private int startOffset = 0;
  private int endOffset = 0;
  private final Stack<ContextType> contexts = new Stack<ContextType>();

  private Scanner(Source origin, String source) {
    pushContext(ContextType.TOP);
    this.origin = origin;
    this.source = source;
    skipWhiteSpace();
  }

  private void pushContext(ContextType type) {
    contexts.push(type);
  }

  private ContextType getContext() throws SyntaxError {
    if (contexts.isEmpty())
      fail();
    return contexts.peek();
  }

  private void popContext() {
    contexts.pop();
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
    switch (Character.toLowerCase(c)) {
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9': case 'a': case 'b':
    case 'c': case 'd': case 'e': case 'f':
      return true;
    default:
      return false;
    }
  }

  private static boolean isOperator(char c) {
    switch (c) {
    case '+': case '-': case '.': case '@': case '%': case '*':
    case '>': case '<': case '!': case '?': case '=': case '|':
    case '&': case '/':
      return true;
    default:
      return false;
    }
  }

  private boolean atPair(char first, char second) {
    return (getCurrent() == first) && peekAhead(second);
  }

  private boolean peekAhead(char value) {
    return (cursor + 1) < source.length() && source.charAt(cursor + 1) == value;
  }

  private void skipBlockComment() {
    assert getCurrent() == '/';
    advance();
    assert getCurrent() == '*';
    while (hasMore() && !atPair('*', '/'))
      advance();
    if (hasMore()) {
      assert getCurrent() == '*';
      advance();
      assert getCurrent() == '/';
      advance();
    }
  }

  private void skipEndOfLineComment() {
    assert getCurrent() == '/';
    advance();
    assert getCurrent() == '/';
    advance();
    while (hasMore() && getCurrent() != '\n')
      advance();
  }

  /**
   * Skip over zero or more characters for whom {@link #isWhitespace(char)}
   * returns true.
   */
  private void skipWhiteSpace() {
    while (hasMore()) {
      if (isWhitespace(getCurrent())) {
        advance();
      } else if (atPair('/', '*')) {
        skipBlockComment();
      } else if (atPair('/', '/')) {
        skipEndOfLineComment();
      } else {
        break;
      }
    }
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
    throw new SyntaxError(origin, token);
  }

  /**
   * Scan over and return the next token in the input.
   */
  private Token.Type scanNext() throws SyntaxError {
    if (getContext() == ContextType.INTERPOL) {
      return scanNextInterpol();
    } else {
      return scanNextPlain();
    }
  }

  private Token.Type scanNextInterpol() throws SyntaxError {
    if (getCurrent() == '}') {
      return scanString(false);
    } else {
      return scanNextPlain();
    }
  }

  private Token.Type scanNextPlain() throws SyntaxError {
    char c = getCurrent();
    switch (c) {
    case '(':
      advance();
      pushContext(ContextType.DELIM);
      return Token.Type.LPAREN;
    case ')':
      advance();
      popContext();
      return Token.Type.RPAREN;
    case '{':
      advance();
      pushContext(ContextType.DELIM);
      return Token.Type.LBRACE;
    case '}':
      advance();
      popContext();
      return Token.Type.RBRACE;
    case '[':
      advance();
      pushContext(ContextType.DELIM);
      return Token.Type.LBRACK;
    case ']':
      advance();
      popContext();
      return Token.Type.RBRACK;
    case '#':
      advance();
      return Token.Type.HASH;
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
      return scanString(true);
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

  private Token.Type scanString(boolean isPlain) {
    assert !isPlain || getCurrent() == '"';
    assert isPlain || getCurrent() == '}';
    startOffset = 1;
    advance();
    while (hasMore() && (getCurrent() != '"') && !atPair('$', '{'))
      advance();
    if (hasMore()) {
      if (getCurrent() == '"') {
        endOffset = -1;
        advance();
        if (isPlain) {
          return Token.Type.STRING;
        } else {
          popContext();
          return Token.Type.INTERPOL_END;
        }
      } else {
        assert atPair('$', '{');
        endOffset = -2;
        advance();
        advance();
        if (isPlain) {
          pushContext(ContextType.INTERPOL);
          return Token.Type.INTERPOL_START;
        } else {
          return Token.Type.INTERPOL_PART;
        }
      }
    } else {
      return Token.Type.ERROR;
    }
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
    while (hasMore() && (isDigit(getCurrent()) || (getCurrent() == 'r')))
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
      tokens.add(new Token(nextType, text, start + startOffset, end));
      skipWhiteSpace();
    }
    return tokens;
  }

  public static List<Token> tokenize(Source origin, String source) throws SyntaxError {
    return new Scanner(origin, source).scan();
  }

}
