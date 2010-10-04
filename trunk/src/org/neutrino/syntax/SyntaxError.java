package org.neutrino.syntax;


public class SyntaxError extends Exception {

  private final Token token;
	private Token.Type expected;

  public SyntaxError(Token token) {
  	this(token, null);
  }

  public SyntaxError(Token token, Token.Type expected) {
    super(token.getValue());
    this.token = token;
	  this.expected = expected;
  }
  
  public Token getOffendingToken() {
    return token;
  }

  @Override
  public String toString() {
  	StringBuilder b = new StringBuilder("Syntax error");
  	b.append(" at ").append(token);
  	if (expected != null) {
  		b.append(", expected ").append(expected);
  	}
  	return b.toString();
  }
}
