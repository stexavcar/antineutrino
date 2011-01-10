package org.javatrino.ast;

import java.util.List;

public class Method {

  private final List<Pattern> signature;
  private final boolean isOpen;
  private final Expression body;

  public Method(List<Pattern> signature, boolean isOpen, Expression body) {
    this.signature = signature;
    this.isOpen = isOpen;
    this.body = body;
  }

}
