package org.neutrino.syntax;

import org.neutrino.compiler.Source;


@SuppressWarnings("serial")
public class SyntaxError extends Exception {

  private final Source origin;
  private final Token token;
  private Token.Type expected;

  public SyntaxError(Source origin, Token token) {
    this(origin, token, null);
  }

  public SyntaxError(Source origin, Token token, Token.Type expected) {
    super(token.getValue());
    this.origin = origin;
    this.token = token;
    this.expected = expected;
  }

  public String getFileName() {
    return origin.getName();
  }

  public int getLineNumber() {
    int start = token.getStart();
    String source = origin.getContents();
    int line = 1;
    for (int i = 0; i < start; i++) {
      if (source.charAt(i) == '\n')
        line++;
    }
    return line;
  }

  public String[] getSourceUnderline() {
    int start = token.getStart();
    String source = origin.getContents();
    int prevNewline = 0;
    int nextNewline = 0;
    for (int i = 0; i < source.length(); i++) {
      if (source.charAt(i) == '\n') {
        if (i <= start) {
          prevNewline = i + 1;
        } else if (i > start) {
          nextNewline = i;
          break;
        }
      }
    }
    if (nextNewline == 0)
      nextNewline = source.length();
    String precedingLine = source.substring(prevNewline, nextNewline);
    StringBuilder buf = new StringBuilder();
    for (int i = prevNewline; i < start; i++) {
      if (source.charAt(i) == '\t')
        buf.append("\t");
      else
        buf.append(" ");
    }
    for (int i = start; i < token.getEnd(); i++)
      buf.append("^");
    return new String[] { precedingLine, buf.toString() };
  }



  public Token getOffendingToken() {
    return token;
  }

  @Override
  public String getMessage() {
    StringBuilder b = new StringBuilder("Syntax error");
    b.append(" at ").append(token);
    if (expected != null) {
      b.append(", expected ").append(expected);
    }
    return b.toString();
  }

}
