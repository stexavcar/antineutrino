package org.neutrino.main;

import org.neutrino.compiler.CompilerModule;
import org.neutrino.compiler.CompilerUniverse;
import org.neutrino.oldton.PSeed;
import org.neutrino.oldton.PValue;
import org.neutrino.pib.BinaryBuilder;
import org.neutrino.pib.Module;
import org.neutrino.pib.Universe;
import org.neutrino.runtime.Interpreter;
import org.neutrino.runtime.RValue;
import org.neutrino.syntax.Parser;
import org.neutrino.syntax.Scanner;
import org.neutrino.syntax.SyntaxError;
import org.neutrino.syntax.Token;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.List;

public class Shell {

  public static @Flags.Flag("pib-path") String pibPath;

  public static void main(String[] rawArgs) throws IOException {
    Flags.parseArguments(rawArgs, Shell.class);
    assert pibPath != null : "No --pib-path has been specified";
    File file = new File(pibPath);
    PValue value = Universe.getOldton().read(new FileInputStream(file));
    Universe universe = ((PSeed) value).grow(Universe.class);
    universe.initialize();
    (new Shell(universe)).readEvalPrint();
  }

  private static final String SHORT_NAME = "shell";
  private static final String LONG_NAME = "org::neutrino::shell";
  private static final String DUMMY_VAR_NAME = "__shell_result__";

  private final BufferedReader in;
  private final Interpreter inter = new Interpreter();
  private final Module shellModule;
  private final Universe universe;

  private Shell(Universe universe) {
    in = new BufferedReader(new InputStreamReader(System.in));
    this.shellModule = universe.getModule(LONG_NAME);
    this.universe = universe;
  }

  public String readLine() throws IOException {
    System.out.print("n> ");
    return in.readLine();
  }

  private void readEvalPrint() throws IOException {
    while (true) {
      String next = readLine();
      if (next == null) {
        System.out.println("");
        return;
      }
      try {
        RValue result = eval(next);
        if (result != null)
          System.out.println(result);
      } catch (SyntaxError se) {
        se.printStackTrace();
      } catch (AssertionError ae) {
        ae.printStackTrace();
      }
    }
  }

  private RValue eval(String source) throws SyntaxError {
    // Create a new instance of the shell module, living on top of the
    // shell module provided by lib.
    CompilerModule module = CompilerModule.createToplevel().ensureModule(SHORT_NAME, LONG_NAME);
    boolean isDeclaration = isDeclaration(source);
    if (isDeclaration) {
      module.includeSource("shell", source);
    } else {
      // If the source is not a declaration we wrap it in a dummy one.
      module.includeSource("shell", "def " + DUMMY_VAR_NAME + " := (" + source + ");");
    }
    // Compile the declaration into a parallel universe.
    module.parseAll();
    BinaryBuilder binary = Universe.builder(new CompilerUniverse(module));
    module.writeToBinary(binary);
    Universe subUniverse = binary.getResult();
    subUniverse.initialize();
    subUniverse.setParallelUniverse(universe);
    if (isDeclaration) {
      // If the input was a declaration we destructively update the
      // state of the persistent shell module.
      Module subShellModule = subUniverse.getModule(LONG_NAME);
      shellModule.defs.putAll(subShellModule.defs);
      shellModule.methods.addAll(subShellModule.methods);
      shellModule.protos.putAll(subShellModule.protos);
      return null;
    } else {
      // If the input was an expression we fetch the value of the
      // dummy variable which will cause the declaration to be evaluated.
      return subUniverse.getGlobal(DUMMY_VAR_NAME, inter);
    }
  }

  private boolean isDeclaration(String source) throws SyntaxError {
    List<Token> tokens = Scanner.tokenize(null, source);
    boolean isDeclaration;
    try {
      Parser.parse(null, tokens);
      isDeclaration = true;
    } catch (SyntaxError se) {
      isDeclaration = false;
    }
    return isDeclaration;
  }

}
