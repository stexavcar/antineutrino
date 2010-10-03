package org.neutrino.pib;

import java.util.List;

import org.neutrino.syntax.Annotation;


public class CodeBuilder {

  private final List<Annotation> annots;
  private final Assembler assm;

  public CodeBuilder(List<Annotation> annots) {
    this.annots = annots;
    this.assm = new Assembler();
  }

  public Assembler getAssembler() {
    return this.assm;
  }

  public List<Annotation> getAnnotations() {
    return annots;
  }

  public Binding getBinding() {
    return new Binding(annots, assm.getCode());
  }

}
