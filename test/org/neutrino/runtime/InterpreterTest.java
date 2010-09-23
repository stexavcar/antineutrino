package org.neutrino.runtime;
import junit.framework.TestCase;

import org.neutrino.compiler.CompilerModule;
import org.neutrino.main.Compile;
import org.neutrino.pib.Universe;
import org.neutrino.syntax.SyntaxError;


public class InterpreterTest extends TestCase {

  private void runTest(Object expected, String expr) {
    CompilerModule top = CompilerModule.createToplevel();
    CompilerModule test = top.ensureModule("test", "test");
    String source = "@entry_point def result := " + expr + ";";
    test.includeSource("test", source);
    Universe bin = null;
    try {
      bin = Compile.compile(top);
    } catch (SyntaxError se) {
      fail(se.getMessage());
    }
    RLambda entryPoint = bin.getEntryPoint();
    Interpreter inter = new Interpreter();
    Object result = inter.interpret(entryPoint);
    assertEquals(expected, result.toString());
  }

  public void testConstant() {
    runTest("4", "4");
    runTest("0", "0");
  }

  public void testLambda() {
    runTest("#<a Lambda>", "fn () -> 4");
  }

}
