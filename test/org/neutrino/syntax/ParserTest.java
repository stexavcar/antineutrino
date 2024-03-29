package org.neutrino.syntax;

import java.util.List;

import junit.framework.TestCase;

import org.neutrino.syntax.Tree.Unit;

/**
 * Unit test of the {@link Parser} class.
 *
 * @author plesner@google.com (Christian Plesner Hansen)
 */
public class ParserTest extends TestCase {

  private static void parserTest(String source, String expected) {
    try {
      List<Token> tokens = Scanner.tokenize(null, source);
      Unit tree = Parser.parse(null, tokens);
      assertEquals(expected, tree.toString());
    } catch (SyntaxError se) {
      throw new RuntimeException(se);
    }
  }

  private static void errorTest(String source, String token) {
    try {
      List<Token> tokens = Scanner.tokenize(null, source);
      Parser.parse(null, tokens);
      fail();
    } catch (SyntaxError se) {
      assertEquals(token, se.getOffendingToken().getValue());
    }
  }

  private void statementTest(String source, String expected) {
    parserTest(
        "def foo() {" + source + "}",
        "(unit (fn #'foo' #'()' () " + expected + "))");
  }

  private void toplevelTest(String source, String expected) {
    parserTest(source, "(unit " + expected + ")");
  }

  private void expressionTest(String source, String expected) {
    statementTest("(" + source + ");", expected);
  }

  public void testSimpleDeclarations() {
    parserTest("def x := y;", "(unit (def #'x' $#'y'))");
    parserTest("def z := 4;", "(unit (def #'z' #4))");
  }

  public void testShortFunctionDeclaration() {
    parserTest("def foo => 3;", "(unit (fn #'foo' #'()' () #3))");
    parserTest("def foo() => 3;", "(unit (fn #'foo' #'()' () #3))");
    parserTest("def foo(a) => 3;", "(unit (fn #'foo' #'()' (a) #3))");
    parserTest("def foo(a, b) => 3;", "(unit (fn #'foo' #'()' (a b) #3))");
    parserTest("def foo() { }", "(unit (fn #'foo' #'()' () #n))");
    errorTest("def foo(a, b => 3;", "=>");
    errorTest("def foo(a, b,) => 3;", ")");
    errorTest("def foo(a, , b) => 3;", ",");
  }

  public void testSimpleStatementBlocks() {
    statementTest(" ", "#n");
    errorTest("def foo { ; }", ";");
    errorTest("def foo { ; ; }", ";");
    errorTest(" 1 ", "1");
    errorTest("def foo { ; 1 }", ";");
    statementTest(" 1; ", "#1");
    errorTest(" 1; 2 ", "1");
    errorTest("def foo { 1;; 2 }", ";");
    statementTest(" 1; 2; ", "(: #1 #2)");
    statementTest(" 1; 2; 3; 4; ", "(: #1 #2 #3 #4)");
  }

  public void testLocalDefinitions() {
    statementTest("def x := 4;", "(let (x #4) #n)");
    errorTest("def x := 4; 8", "8");
    errorTest("def x := 5;; 6", ";");
    errorTest("def x := 4; 8; 9; 10", "8");
    statementTest("1; def a := 9; 3; 4;", "(: #1 (let (a #9) (: #3 #4)))");
  }

  public void testProtocol() {
    toplevelTest("protocol Foo;", "(protocol #'Foo')");
    toplevelTest("@native protocol Bar;", "(protocol (@ native) #'Bar')");
  }

  public void testAnnotation() {
    toplevelTest("@annot def x := 4;", "(def (@ annot) #'x' #4)");
    toplevelTest("@a @b @c def y := 4;", "(def (@ a b c) #'y' #4)");
    toplevelTest("@annot() def z := 4;", "(def (@ annot) #'z' #4)");
    toplevelTest("@annot(1) def z := 4;", "(def (@ (annot #1)) #'z' #4)");
    toplevelTest("@annot(1, 2) def z := 4;", "(def (@ (annot #1 #2)) #'z' #4)");
    toplevelTest("@annot(1, 2, 3) def z := 4;", "(def (@ (annot #1 #2 #3)) #'z' #4)");
  }

  public void testSingletons() {
    expressionTest("true", "#t");
    expressionTest("false", "#f");
    expressionTest("null", "#n");
  }

  public void testStrings() {
    expressionTest("\"foo\"", "#'foo'");
  }

  public void testMethodCalls() {
    expressionTest("a.b()", "(.b $#'a')");
    expressionTest("a.b().c()", "(.c (.b $#'a'))");
    expressionTest("a.b(1)", "(.b $#'a' #1)");
    expressionTest("a.b(1, 2)", "(.b $#'a' #1 #2)");
    expressionTest("a.b(1, 2, 3)", "(.b $#'a' #1 #2 #3)");
    expressionTest("a.b.c.d.e", "(..e (..d (..c (..b $#'a'))))");
    expressionTest("a.b().c.d().e", "(..e (.d (..c (.b $#'a'))))");
  }

  public void testFunctionInvocation() {
    expressionTest("a()", "(.() $#'a')");
    expressionTest("(a.b)()", "(.() (..b $#'a'))");
    expressionTest("a.b()", "(.b $#'a')");
  }

  public void testInfixOperator() {
    expressionTest("1 + 1", "(.+ #1 #1)");
    expressionTest("1 + 2 + 3", "(.+ (.+ #1 #2) #3)");
    expressionTest("1 ** 2 ++ 3", "(.++ (.** #1 #2) #3)");
  }

}
