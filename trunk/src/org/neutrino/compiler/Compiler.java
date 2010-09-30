package org.neutrino.compiler;

import org.neutrino.pib.Assembler;
import org.neutrino.pib.CodeBundle;
import org.neutrino.syntax.Tree;

public class Compiler {

  public static CodeBundle compile(Assembler assm, Tree.Expression body) {
    LexicalAnalyzer lexicizer = new LexicalAnalyzer();
    body.accept(lexicizer);
    CodeGenerator codegen = new CodeGenerator(assm);
    codegen.generate(body);
    return assm.getCode();
  }

  public static void compileNativeLambda(Assembler assm, Tree.Definition that) {
    CodeGenerator codegen = new CodeGenerator(assm);
    codegen.generateNativeLambda(that);
  }

  public static void compileNativeMethod(Assembler assm, Tree.Method that) {
    CodeGenerator codegen = new CodeGenerator(assm);
    codegen.generateNativeMethod(that);
  }

}
