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
      List<Token> tokens = Scanner.tokenize(source);
      Unit tree = Parser.parse(tokens);
      assertEquals(expected, tree.toString());
    } catch (SyntaxError se) {
      fail(se.toString());
    }
  }

  private static void errorTest(String source, String token) {
    try {
      List<Token> tokens = Scanner.tokenize(source);
      Parser.parse(tokens);
      fail();
    } catch (SyntaxError se) {
      assertEquals(token, se.getOffendingToken().getValue());
    }
  }

  public void testSimpleDeclarations() {
    parserTest("def x := y;", "(unit (def x $y))");
    parserTest("def z := 4;", "(unit (def z #4))");
  }

  public void testShortFunctionDeclaration() {
    parserTest("def foo -> 3;", "(unit (def foo (fn () #3)))");
    parserTest("def foo() -> 3;", "(unit (def foo (fn () #3)))");
    parserTest("def foo(a) -> 3;", "(unit (def foo (fn (a) #3)))");
    parserTest("def foo(a, b) -> 3;", "(unit (def foo (fn (a b) #3)))");
    errorTest("def foo(a, b -> 3;", "->");
    errorTest("def foo(a, b,) -> 3;", ")");
    errorTest("def foo(a, , b) -> 3;", ",");
  }

  private void statementTest(String source, String expected) {
    parserTest(
        "def foo() {" + source + "}",
        "(unit (def foo (fn () " + expected + ")))");
  }

  public void testSimpleStatementBlocks() {
    statementTest(" ", "null");
    errorTest("def foo { ; }", ";");
    errorTest("def foo { ; ; }", ";");
    statementTest(" 1 ", "#1");
    errorTest("def foo { ; 1 }", ";");
    statementTest(" 1; ", "#1");
    statementTest(" 1; 2 ", "(: #1 #2)");
    errorTest("def foo { 1;; 2 }", ";");
    statementTest(" 1; 2; ", "(: #1 #2)");
    errorTest("def foo { 1; 2;; }", ";");
    statementTest(" 1; 2; 3; 4; ", "(: #1 #2 #3 #4)");
  }

  public void testLocalDefinitions() {
    statementTest("def x := 4;", "(let (x #4) null)");
    statementTest("def x := 4; 8", "(let (x #4) #8)");
    errorTest("def x := 5;; 6", ";");
    statementTest("def x := 4; 8; 9; 10", "(let (x #4) (: #8 #9 #10))");
    statementTest("1; def a := 9; 3; 4;", "(: #1 (let (a #9) (: #3 #4)))");
  }

  public void testAnnotation() {
    parserTest("@annot def x := 4;", "(unit (def (@ annot) x #4))");
    parserTest("@a @b @c def y := 4;", "(unit (def (@ a b c) y #4))");
  }

}
