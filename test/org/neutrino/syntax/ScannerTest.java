package org.neutrino.syntax;

import java.util.Arrays;

import junit.framework.TestCase;

import org.neutrino.syntax.Token.Type;

/**
 * Unit test of the {@link Scanner} class.
 *
 * @author plesner@google.com (Christian Plesner Hansen)
 */
public class ScannerTest extends TestCase {

  private static Token ident(String value) {
    return new Token(Type.IDENT, value, 0, 0);
  }

  private static Token number(String value) {
    return new Token(Type.NUMBER, value, 0, 0);
  }

  private static Token string(String value) {
    return new Token(Type.STRING, value, 0, 0);
  }

  private static Token op(String value) {
    return new Token(Type.OPERATOR, value, 0, 0);
  }

  private static Token newToken(Token.Type type) {
    return new Token(type, type.getName(), 0, 0);
  }

  private static final Token LPAREN = newToken(Type.LPAREN);
  private static final Token RPAREN = newToken(Type.RPAREN);
  private static final Token LBRACE = newToken(Type.LBRACE);
  private static final Token RBRACE = newToken(Type.RBRACE);
  private static final Token SEMI = newToken(Type.SEMI);
  private static final Token COOLON = newToken(Type.COOLON);
  private static final Token DOT = newToken(Type.DOT);
  private static final Token COMMA = newToken(Type.COMMA);
  private static final Token COLON = newToken(Type.COLON);
  private static final Token COLON_EQ = newToken(Type.COLON_EQ);
  private static final Token ARROW = newToken(Type.ARROW);
  private static final Token DEF = newToken(Type.DEF);
  private static final Token FN = newToken(Type.FN);
  private static final Token NEW = newToken(Type.NEW);
  private static final Token NULL = newToken(Type.NULL);
  private static final Token TRUE = newToken(Type.TRUE);
  private static final Token FALSE = newToken(Type.FALSE);
  private static final Token PROTOCOL = newToken(Type.PROTOCOL);

  private void scannerTest(String input, Token... expected) {
    try {
      assertEquals(Arrays.asList(expected), Scanner.tokenize(input));
    } catch (SyntaxError se) {
      fail(se.toString());
    }
  }

  public void testSimple() {
    scannerTest("foo bar baz", ident("foo"), ident("bar"), ident("baz"));
    scannerTest(" foo bar baz ", ident("foo"), ident("bar"), ident("baz"));
    scannerTest("(foo)(bar)(baz)", LPAREN, ident("foo"), RPAREN,
        LPAREN, ident("bar"), RPAREN, LPAREN, ident("baz"), RPAREN);
    scannerTest("def fn new null true false protocol", DEF, FN, NEW, NULL, TRUE, FALSE, PROTOCOL);
    scannerTest("12", number("12"));
  }

  public void testStrings() {
    scannerTest("\"foo\"", string("foo"));
    scannerTest("\"\"", string(""));
  }

  public void testDelimiters() {
    scannerTest("; : := , . -> ::", SEMI, COLON,
        COLON_EQ, COMMA, DOT, ARROW, COOLON);
    scannerTest(",;:.", COMMA, SEMI, COLON, DOT);
    scannerTest("{}", LBRACE, RBRACE);
  }

  public void testOperator() {
    scannerTest("++ -- *?", op("++"), op("--"), op("*?"));
  }

}
