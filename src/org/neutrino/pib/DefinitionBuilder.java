package org.neutrino.pib;

import java.util.List;


public class DefinitionBuilder {

  private final Assembler assm;

  public DefinitionBuilder(List<String> annots) {
    this.assm = new Assembler(annots);
  }

  public Assembler getAssembler() {
    return this.assm;
  }

}
