package org.neutrino.pib;

import java.util.List;


public class DefinitionBuilder {

  private final List<String> annots;
  private final Assembler assm;

  public DefinitionBuilder(List<String> annots) {
    this.annots = annots;
    this.assm = new Assembler();
  }

  public Assembler getAssembler() {
    return this.assm;
  }

  public Binding getBinding() {
    return new Binding(annots, assm.getCode());
  }

}
