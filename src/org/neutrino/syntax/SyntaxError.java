package org.neutrino.syntax;

public class SyntaxError extends Exception {

  private final Token token;

  public SyntaxError(Token token) {
    super(token.getValue());
    this.token = token;
  }

  public Token getOffendingToken() {
    return token;
  }

}
