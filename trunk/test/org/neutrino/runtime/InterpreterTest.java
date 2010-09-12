package org.neutrino.runtime;
import junit.framework.TestCase;

import org.neutrino.compiler.Module;
import org.neutrino.main.Compile;
import org.neutrino.pib.Binary;
import org.neutrino.pib.Binding;
import org.neutrino.syntax.SyntaxError;


public class InterpreterTest extends TestCase {

  private void runTest(Object expected, String expr) {
    Module top = Module.createToplevel();
    Module test = top.ensureModule("test", "test");
    String source = "@entry_point def result := " + expr + ";";
    test.includeSource("test", source);
    Binary bin = null;
    try {
      bin = Compile.compile(top);
    } catch (SyntaxError se) {
      fail(se.getMessage());
    }
    Binding entryPoint = bin.getEntryPoint();
    Interpreter inter = new Interpreter();
    Object result = inter.interpret(entryPoint.getCode());
    assertEquals(expected, result.toString());
  }

  public void testConstant() {
    runTest("4", "4");
    runTest("0", "0");
  }

  public void testLambda() {
    runTest("#<a Lambda>", "fn () -> 4");
  }

  public void testCalls() {
    // runTest("2", "1 + 1");
  }

}
