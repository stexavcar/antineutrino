package org.neutrino.compiler;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.javatrino.ast.Method;
import org.javatrino.ast.Pattern;
import org.javatrino.ast.Test;
import org.javatrino.ast.Test.Any;
import org.javatrino.ast.Test.Eq;
import org.javatrino.ast.Test.Is;
import org.neutrino.pib.Binding;
import org.neutrino.pib.CodeBundle;
import org.neutrino.pib.Module;
import org.neutrino.pib.Parameter;
import org.neutrino.runtime.TypeId;
import org.neutrino.syntax.Parser;
import org.neutrino.syntax.Scanner;
import org.neutrino.syntax.SyntaxError;
import org.neutrino.syntax.Token;
import org.neutrino.syntax.Tree;
import org.neutrino.syntax.Tree.Definition;
import org.neutrino.syntax.Tree.Inheritance;
import org.neutrino.syntax.Tree.Protocol;

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

  public Tree.Method findMethod(String holder, String name) {
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

  private class StaticBuildingVisitor implements Tree.DeclarationVisitor {

    private final Module module;

    public StaticBuildingVisitor(Module module) {
      this.module = module;
    }

    public void visitProtocol(Protocol that) {
      module.createProtocol(Source.this, that.getAnnotations(),
          that.getName(), that.getName());
    }

    @Override
    public void visitDefinition(Definition that) {
      // ignore -- for now
    }

    @Override
    public void visitInheritance(Inheritance that) {
      // ignore
    }

    @Override
    public void visitMethodDefinition(org.neutrino.syntax.Tree.Method that) {
      // ignore
    }

  }

  private class BuildingVisitor implements Tree.DeclarationVisitor {

    private final Module module;

    public BuildingVisitor(Module module) {
      this.module = module;
    }

    public void visitDefinition(Definition that) {
      CodeBundle bundle = Compiler.compile(module, Source.this, that.getValue(),
          null);
      Binding binding = new Binding(that.getAnnotations(), bundle);
      module.createDefinition(that.getName(), binding);
    }

    public void visitProtocol(Protocol that) {
      // ignore
    }

    public void visitMethodDefinition(Tree.Method that) {
      CodeBundle bundle = Compiler.compile(module, Source.this, that.getBody(),
          that.getParameters());
      List<Pattern> signature = new ArrayList<Pattern>();
      signature.add(new Pattern(
          Arrays.<Object>asList("name"),
          new Test.Eq(that.getMethodName()),
          null));
      int index = 0;
      for (Parameter param : that.getParameters()) {
        Test test;
        if ("Object".equals(param.type)) {
          test = new Any();
        } else if (param.isProtocolMethod) {
          TypeId type = param.ensureTypeId(module);
          test = new Eq(type);
        } else {
          TypeId type = param.ensureTypeId(module);
          test = new Is(type);
        }
        signature.add(new Pattern(Arrays.<Object>asList(index), test, param.getSymbol()));
        index++;
      }
      module.createMethod(new Method(that.getAnnotations(),signature, false,
          bundle.body, bundle.rewrites, module));
    }

    public void visitInheritance(Inheritance that) {
      module.declareInheritance(that.getSub(), that.getSuper());
    }

  }

  public void writeTo(Module module) {
    BuildingVisitor visitor = new BuildingVisitor(module);
    for (Tree.Declaration decl : code.getDeclarations())
      decl.accept(visitor);
  }

  public void writeStatics(Module module) {
    StaticBuildingVisitor visitor = new StaticBuildingVisitor(module);
    for (Tree.Declaration decl : code.getDeclarations())
      decl.accept(visitor);
  }

}
