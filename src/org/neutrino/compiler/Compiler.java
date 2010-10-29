package org.neutrino.compiler;

import org.neutrino.pib.Assembler;
import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Expression;
import org.neutrino.syntax.Tree.Method;

public class Compiler {

  public static CodeBundle compile(ModuleBuilder module,
      Assembler assm, Tree.Expression body) {
    LexicalAnalyzer lexicizer = new LexicalAnalyzer();
    body.accept(lexicizer);
    ImplicitDeclarationVisitor implicitor = new ImplicitDeclarationVisitor(module);
    body.accept(implicitor);
    CodeGenerator codegen = new CodeGenerator(module.getUniverse(), assm);
    int rootOffset = codegen.generate(body);
    assm.finalize(0, rootOffset);
    return assm.getCode();
  }

  public static void compileMethod(ModuleBuilder module,
      Assembler assm, Method that) {
    Expression body = that.getBody();
    LexicalAnalyzer lexicizer = new LexicalAnalyzer(that);
    body.accept(lexicizer);
    ImplicitDeclarationVisitor implicitor = new ImplicitDeclarationVisitor(module);
    body.accept(implicitor);
    CodeGenerator codegen = new CodeGenerator(module.getUniverse(), assm);
    int rootOffset = codegen.generate(body);
    assm.finalize(lexicizer.getLocalCount(), rootOffset);
  }

}
