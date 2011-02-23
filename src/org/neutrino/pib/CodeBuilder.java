package org.neutrino.pib;

import java.util.List;

import org.javatrino.ast.Expression;
import org.javatrino.ast.Symbol;
import org.neutrino.compiler.Source;
import org.neutrino.syntax.Annotation;


public class CodeBuilder {

  private final List<Annotation> annots;
  private final Assembler assm;

  public CodeBuilder(Source origin, List<Annotation> annots) {
    this.annots = annots;
    this.assm = new Assembler(origin);
  }

  public Assembler getAssembler() {
    return this.assm;
  }

  public Assembler getAssembler(Expression expr, List<Symbol> params) {
    this.assm.finalize(expr, params);
    return this.assm;
  }

  public List<Annotation> getAnnotations() {
    return annots;
  }

  public Binding getBinding() {
    return new Binding(annots, assm.getCode());
  }

}
