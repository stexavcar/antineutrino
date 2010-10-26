package org.neutrino.compiler;

import org.neutrino.pib.CodeBuilder;
import org.neutrino.pib.ModuleBuilder;
import org.neutrino.pib.Parameter;
import org.neutrino.syntax.Parser;
import org.neutrino.syntax.Scanner;
import org.neutrino.syntax.SyntaxError;
import org.neutrino.syntax.Token;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Definition;
import org.neutrino.syntax.Tree.Inheritance;
import org.neutrino.syntax.Tree.Method;
import org.neutrino.syntax.Tree.Protocol;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.List;

/**
 * A single source file in a module.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Source {

  private static final String EXTENSION = ".n";
  private final String name;
  private final String contents;
  private Tree.Unit code;

  private Source(String name, String contents) {
    this.name = name;
    this.contents = contents;
  }

  public Tree.Declaration findDeclaration(String target) {
    for (Tree.Declaration decl : code.getDeclarations()) {
      if (target.equals(decl.getName()))
        return decl;
    }
    return null;
  }

  public Method findMethod(String holder, String name) {
    for (Tree.Declaration decl : code.getDeclarations()) {
      if (!(decl instanceof Tree.Method))
        continue;
      Tree.Method method = (Tree.Method) decl;
      if (!method.getMethodName().equals(name))
        continue;
      for (Parameter param : method.getParameters()) {
        if (param.isProtocolMethod && param.type.equals(holder))
          return method;
      }
    }
    return null;
  }

  public String getName() {
    return this.name;
  }

  public String getContents() {
    return contents;
  }

  void ensureParsed() throws SyntaxError {
    if (this.code == null) {
      List<Token> tokens = Scanner.tokenize(this, contents);
      this.code = Parser.parse(this, tokens);
    }
  }

  /**
   * Creates and returns a source object for the source in the given
   * file or, if the file is not a neutrino source file, returns null.
   */
  public static Source create(File file) throws IOException {
    assert file.exists() && file.isFile();
    String fileName = file.getName();
    if (!fileName.endsWith(EXTENSION))
      return null;
    String contents = readFile(file);
    return create(file.getPath(), contents);
  }

  public static Source create(String name, String str) {
    return new Source(name, str);
  }

  private static String readFile(File file) throws IOException {
    FileReader in = new FileReader(file);
    StringBuilder buf = new StringBuilder();
    char[] block = new char[1024];
    while (in.ready()) {
      int count = in.read(block);
      if (count > 0) {
        buf.append(block, 0, count);
      } else {
        break;
      }
    }
    return buf.toString();
  }

  @Override
  public String toString() {
    return "source " + name;
  }

  private class BuildingVisitor implements Tree.DeclarationVisitor {

    private final ModuleBuilder module;

    public BuildingVisitor(ModuleBuilder module) {
      this.module = module;
    }

    public void visitDefinition(Definition that) {
      CodeBuilder builder = module.createDefinition(Source.this, that);
      Compiler.compile(module, builder.getAssembler(), that.getValue());
    }

    public void visitProtocol(Protocol that) {
      module.createProtocol(Source.this, that.getAnnotations(),
          that.getName(), that.getName());
    }

    public void visitMethodDefinition(Method that) {
      CodeBuilder builder = module.createMethod(Source.this, that.getAnnotations(),
          that.getMethodName(), that.getParameters());
      Compiler.compileMethod(module, builder.getAssembler(), that);
    }

    public void visitInheritance(Inheritance that) {
      module.declareInheritance(that.getSub(), that.getSuper());
    }

  }

  public void writeToBinary(ModuleBuilder module) {
    BuildingVisitor visitor = new BuildingVisitor(module);
    for (Tree.Declaration decl : code.getDeclarations())
      decl.accept(visitor);
  }

}
