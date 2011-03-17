package org.neutrino.syntax;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.neutrino.compiler.Source;
import org.neutrino.pib.Parameter;
import org.neutrino.syntax.Token.Type;
import org.neutrino.syntax.Tree.If;
import org.neutrino.syntax.Tree.Lambda;

/**
 * Plain vanilla recursive descent parser for the neutrino syntax.
 * Main entry point is {@link #parse(Source, List)}.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Parser {

  private final Source source;
  private final List<Token> tokens;
  private int cursor = 0;
  private boolean hasTransientSemicolon = false;

  private Parser(Source source, List<Token> tokens) {
    this.source = source;
    this.tokens = tokens;
  }

  private Token getCurrent() {
    return tokens.get(cursor);
  }

  private void advance() {
    cursor++;
    hasTransientSemicolon = false;
  }

  private boolean hasMore() {
    return cursor < tokens.size();
  }

  private Tree.Unit parseUnit() throws SyntaxError {
    List<Tree.Declaration> defs = new ArrayList<Tree.Declaration>();
    while (hasMore()) {
      List<Tree.Declaration> def = parseDeclaration();
      defs.addAll(def);
    }
    return new Tree.Unit(defs);
  }

  private SyntaxError currentSyntaxError() {
    return new SyntaxError(source, getCurrent());
  }

  private String expect(Type type) throws SyntaxError {
    if (!hasMore())
      throw new SyntaxError(source, new Token(Type.ERROR, "<eof>", 0, 0), type);
    Token current = getCurrent();
    if (!current.getType().equals(type))
      throw new SyntaxError(source, current, type);
    String value = current.getValue();
    advance();
    return value;
  }

  private boolean at(Type type) {
    return hasMore() && (getCurrent().getType() == type);
  }

  private boolean atOperator() {
    return hasMore() && getCurrent().getType().isOperator();
  }

  private String expectOperator() throws SyntaxError {
    if (!atOperator())
      expect(Type.OPERATOR);
    String result = getCurrent().getValue();
    advance();
    return result;
  }

  boolean atDefinitionStart() {
    return at(Type.DEF) || at(Type.REF);
  }

  boolean consumeDefinitionStart() throws SyntaxError {
    if (at(Type.REF)) {
      expect(Type.REF);
      return true;
    } else {
      expect(Type.DEF);
      return false;
    }
  }

  /**
   * <statement>
   */
  private Tree.Expression parseStatement() throws SyntaxError {
    List<Tree.Annotation> annots = parseAnnotations();
    if (atDefinitionStart()) {
      boolean isReference = consumeDefinitionStart();
      String name = expect(Type.IDENT);
      Tree.Expression value;
      if (at(Type.COLON_EQ)) {
        expect(Type.COLON_EQ);
        value = parseExpression(false);
      } else if (atFunctionDefinitionMarker()) {
        List<Parameter> params = new ArrayList<Parameter>();
        String methodName = parseParameters("()", params, false);
        Tree.Expression body = parseFunctionBody(false);
        value = Tree.Lambda.create(source, methodName, params, body, name);
      } else {
        throw currentSyntaxError();
      }
      Tree.Expression body;
      if (consumeSemicolon()) {
        if (hasMore() && !at(Type.RBRACE)) {
          body = parseStatements();
        } else {
          body = new Tree.Singleton(Tree.Singleton.Type.NULL);
        }
      } else {
        body = new Tree.Singleton(Tree.Singleton.Type.NULL);
      }
      return new Tree.LocalDefinition(annots, name, value, body, isReference);
    } else {
      return parseExpression(true);
    }
  }

  private boolean consumeSemicolon() throws SyntaxError {
    if (at(Type.SEMI)) {
      expect(Type.SEMI);
    } else if (!hasTransientSemicolon) {
      return false;
    }
    return !at(Type.RBRACE);
  }

  private void requireSemicolon() throws SyntaxError {
    if (at(Type.SEMI)) {
      expect(Type.SEMI);
      hasTransientSemicolon = true;
    } else if (!hasTransientSemicolon) {
      throw currentSyntaxError();
    }
  }

  private void checkSemicolon(boolean isStatement) throws SyntaxError {
    if (isStatement)
      requireSemicolon();
  }

  /**
   * <statements>
   *   <statement> +: ";"+ ";"?
   */
  private Tree.Expression parseStatements() throws SyntaxError {
    Tree.Expression expr = parseStatement();
    if (hasMore() && !at(Type.RBRACE)) {
      List<Tree.Expression> exprs = new ArrayList<Tree.Expression>();
      exprs.add(expr);
      while (hasMore() && !at(Type.RBRACE)) {
        Tree.Expression next = parseStatement();
        exprs.add(next);
        if (!consumeSemicolon())
          break;
      }
      return new Tree.Block(exprs);
    } else {
      return expr;
    }
  }

  private void consumeRightBrace() throws SyntaxError {
    expect(Type.RBRACE);
    this.hasTransientSemicolon = true;
  }

  /**
   * <block>
   *   -> "{" <statement> ... "}"
   */
  private Tree.Expression parseBlock() throws SyntaxError {
    expect(Type.LBRACE);
    if (at(Type.RBRACE)) {
      consumeRightBrace();
      return new Tree.Singleton(Tree.Singleton.Type.NULL);
    } else {
      Tree.Expression result = parseStatements();
      consumeRightBrace();
      return result;
    }
  }

  /**
   * <funbody>
   *   -> "->" <expr> ";"
   *   -> <block>
   */
  private Tree.Expression parseFunctionBody(boolean isStatement) throws SyntaxError {
    if (at(Type.ARROW)) {
      expect(Type.ARROW);
      Tree.Expression body = parseExpression(false);
      checkSemicolon(isStatement);
      return body;
    } else if (at(Type.LBRACE)) {
      Tree.Expression body = parseBlock();
      return body;
    } else if (isStatement && at(Type.SEMI)) {
      expect(Type.SEMI);
      return new Tree.Singleton(Tree.Singleton.Type.NULL);
    } else {
      throw currentSyntaxError();
    }
  }

  /**
   * <annots>
   *   -> ("@" <ident>)*
   */
  private List<Tree.Annotation> parseAnnotations() throws SyntaxError {
    List<Tree.Annotation> annots = new ArrayList<Tree.Annotation>();
    while (at(Type.AT)) {
      expect(Type.AT);
      String name = expect(Type.IDENT);
      List<Tree.Expression> args = Collections.emptyList();
      if (at(Type.LPAREN)) {
        expect(Type.LPAREN);
        if (!at(Type.RPAREN)) {
          args = new ArrayList<Tree.Expression>();
          Tree.Expression first = parseExpression(false);
          args.add(first);
          while (at(Type.COMMA)) {
            expect(Type.COMMA);
            Tree.Expression next = parseExpression(false);
            args.add(next);
          }
        }
        expect(Type.RPAREN);
      }
      annots.add(new Tree.Annotation(name, args));
    }
    return annots;
  }

  private boolean atName() {
    switch (getCurrent().getType()) {
    case NEW: case FOR: case IDENT: case IF:
      return true;
    default:
      return atOperator();
    }
  }

  private String expectName(boolean allowEmpty) throws SyntaxError {
    if (atName()) {
      return expect(getCurrent().getType());
    } else if (allowEmpty) {
      return null;
    } else {
      throw currentSyntaxError();
    }
  }

  private boolean atFunctionDefinitionMarker() {
    return at(Type.ARROW) || at(Type.LPAREN) || at(Type.LBRACE) || at(Type.LBRACK);
  }

  private Tree.Method parseMethodTail(List<Tree.Annotation> annots,
      Parameter self) throws SyntaxError {
    if (at(Type.DOT))
      expect(Type.DOT);
    List<Parameter> argParams = new ArrayList<Parameter>();
    String method = parseParameters("()", argParams, true);
    Tree.Expression body = parseFunctionBody(true);
    List<Parameter> params = new ArrayList<Parameter>();
    params.add(self);
    params.addAll(argParams);
    return new Tree.Method(annots, method, params, body);
  }

  private Tree.Declaration parseInnerDefinition(String name) throws SyntaxError {
    List<Tree.Annotation> annots = parseAnnotations();
    expect(Type.DEF);
    String self = expect(Type.IDENT);
    boolean isProtocol = (Annotation.get("static", annots) != null);
    return parseMethodTail(annots, new Parameter(self, name, isProtocol));
  }

  /**
   * <def>
   *   -> <annots> "def" <ident> ":=" <expr> ";"
   *   -> <annots> "def" <ident> "::" <methodname> <params> <funbody>
   *   -> <annots> "def" <ident> <params> <funbody>
   *   -> <annots> "protocol" <ident> ";"
   */
  private List<Tree.Declaration> parseDeclaration() throws SyntaxError {
    List<Tree.Annotation> annots = parseAnnotations();
    if (at(Type.DEF)) {
      expect(Type.DEF);
      if (at(Type.LPAREN)) {
        expect(Type.LPAREN);
        Parameter self = parseParameter();
        expect(Type.RPAREN);
        Tree.Declaration result = parseMethodTail(annots, self);
        return Collections.singletonList(result);
      } else if (at(Type.IDENT)) {
        String name = expect(Type.IDENT);
        if (at(Type.COLON_EQ)) {
          expect(Type.COLON_EQ);
          Tree.Expression value = parseExpression(false);
          requireSemicolon();
          Tree.Declaration result = new Tree.Definition(annots, name, value);
          return Collections.singletonList(result);
        } else if (at(Type.DOT)) {
          Tree.Declaration result = parseMethodTail(annots, new Parameter("this", name, true));
          return Collections.singletonList(result);
        } else if (at(Type.IS)) {
          expect(Type.IS);
          String parent = expect(Type.IDENT);
          requireSemicolon();
          Tree.Declaration result = new Tree.Inheritance(annots, name, parent);
          return Collections.singletonList(result);
        } else if (atFunctionDefinitionMarker()) {
          List<Parameter> params = new ArrayList<Parameter>();
          String methodName = parseParameters("()", params, false);
          Tree.Expression body = parseFunctionBody(true);
          Tree.Expression lambda = Tree.Lambda.create(source, methodName,
              params, body, name);
          Tree.Declaration result = new Tree.Definition(annots, name, lambda);
          return Collections.singletonList(result);
        } else {
          throw currentSyntaxError();
        }
      } else {
        throw currentSyntaxError();
      }
    } else {
      List<Tree.Declaration> result = new ArrayList<Tree.Declaration>();
      expect(Type.PROTOCOL);
      String name = expect(Type.IDENT);
      if (at(Type.IS)) {
        expect(Type.IS);
        String first = expect(Type.IDENT);
        result.add(new Tree.Inheritance(Collections.<Tree.Annotation>emptyList(), name, first));
        while (at(Type.COMMA)) {
          expect(Type.COMMA);
          String next = expect(Type.IDENT);
          result.add(new Tree.Inheritance(Collections.<Tree.Annotation>emptyList(), name, next));
        }
      }
      if (at(Type.LBRACE)) {
        expect(Type.LBRACE);
        while (hasMore() && !at(Type.RBRACE)) {
          result.add(parseInnerDefinition(name));
        }
        consumeRightBrace();
      }
      result.add(new Tree.Protocol(annots, name));
      requireSemicolon();
      return result;
    }
  }

  private Parameter parseParameter() throws SyntaxError {
    String name = expect(Type.IDENT);
    String type = null;
    if (at(Type.IS)) {
      expect(Type.IS);
      type = expect(Type.IDENT);
    }
    return new Parameter(name, type, false);
  }

  private void parseNakedParameters(List<Parameter> params) throws SyntaxError {
    while (true) {
      params.add(parseParameter());
      if (at(Type.COMMA)) {
        expect(Type.COMMA);
        continue;
      } else {
        break;
      }
    }
  }

  private String parseParameters(String name, List<Parameter> params,
      boolean paramSyntaxSignificant) throws SyntaxError {
    String methodName = name;
    if (atName())
      methodName = expectName(false);
    Token.Type begin = Type.LPAREN;
    Token.Type end = Type.RPAREN;
    if (at(Type.LBRACK)) {
      begin = Type.LBRACK;
      end = Type.RBRACK;
      methodName = "[]";
    }
    if (at(begin)) {
      expect(begin);
      if (!at(end))
        parseNakedParameters(params);
      expect(end);
    } else if (paramSyntaxSignificant) {
      methodName = "." + methodName;
    }
    if (at(Type.COLON_EQ)) {
      expect(Type.COLON_EQ);
      expect(Type.LPAREN);
      params.add(parseParameter());
      expect(Type.RPAREN);
      methodName += ":=";
    }
    return methodName;
  }

  /**
   * <arguments>
   *   -> <expr> *: ","
   */
  private List<Tree.Expression> parseArguments(Type end) throws SyntaxError {
    List<Tree.Expression> result = new ArrayList<Tree.Expression>();
    if (hasMore() && !at(end)) {
      Tree.Expression first = parseExpression(false);
      result.add(first);
    }
    while (at(Type.COMMA)) {
      expect(Type.COMMA);
      Tree.Expression next = parseExpression(false);
      result.add(next);
    }
    return result;
  }


  /**
   * <callexpr-tail>
   *   -> "." <ident> ("(" <arguments> ")")?
   *   -> "(" <arguments> ")"
   */
  private Tree.Expression parseCallExpressionTail(Tree.Expression recv) throws SyntaxError {
    String method;
    Type argDelimStart = Type.LPAREN;
    Type argDelimEnd = Type.RPAREN;
    if (at(Type.DOT)) {
      expect(Type.DOT);
      method = expectName(false);
    } else if (at(Type.LPAREN)) {
      method = "()";
    } else if (at(Type.LBRACK)) {
      method = "[]";
      argDelimStart = Type.LBRACK;
      argDelimEnd = Type.RBRACK;
    } else {
      throw currentSyntaxError();
    }
    List<Tree.Expression> args = new ArrayList<Tree.Expression>();
    args.add(recv);
    if (at(argDelimStart)) {
      expect(argDelimStart);
      args.addAll(parseArguments(argDelimEnd));
      expect(argDelimEnd);
    } else {
      method = "." + method;
    }
    return new Tree.Call(method, args);
  }

  private boolean atCallExpressionTail() {
    return at(Type.DOT) || at(Type.LPAREN) || at(Type.LBRACK);
  }

  /**
   * <callexpr>
   *   -> <atomic> (<callexpr-tail>)*
   */
  private Tree.Expression parseCallExpression() throws SyntaxError {
    Tree.Expression current = parseAtomicExpression();
    while (atCallExpressionTail()) {
      current = parseCallExpressionTail(current);
    }
    return current;
  }

  private Tree.Expression parseOperatorExpression() throws SyntaxError {
    Tree.Expression current = parseCallExpression();
    while (atOperator()) {
      String op = expectOperator();
      Tree.Expression next = parseCallExpression();
      current = new Tree.Call(op, Arrays.asList(current, next));
    }
    return current;
  }

  private Tree.Expression parseAssignmentExpression() throws SyntaxError {
    Tree.Expression target = parseOperatorExpression();
    if (at(Type.COLON_EQ)) {
      expect(Type.COLON_EQ);
      Tree.Expression value = parseAssignmentExpression();
      return target.getAssignment(value);
    } else {
      return target;
    }
  }

  private Tree.Expression parseNotExpression() throws SyntaxError {
    if (at(Type.NOT)) {
      expect(Type.NOT);
      Tree.Expression inner = parseNotExpression();
      return new Tree.Call("negate", Collections.singletonList(inner));
    } else {
      return parseAssignmentExpression();
    }
  }

  private Tree.Expression parseAndExpression() throws SyntaxError {
    Tree.Expression current = parseNotExpression();
    while (at(Type.AND)) {
      expect(Type.AND);
      Tree.Expression next = parseNotExpression();
      current = If.create(source, current, next,
          new Tree.Singleton(Tree.Singleton.Type.FALSE));
    }
    return current;
  }

  private Tree.Expression parseOrExpression() throws SyntaxError {
    Tree.Expression current = parseAndExpression();
    while (at(Type.OR)) {
      expect(Type.OR);
      Tree.Expression next = parseAndExpression();
      current = If.create(source, current,
          new Tree.Singleton(Tree.Singleton.Type.TRUE),
          next);
    }
    return current;
  }

  private Tree.Expression parseLogicalExpression() throws SyntaxError {
    return parseOrExpression();
  }

  /**
   * <longexpr>
   *   -> "fn" <args> <funbody>
   *    | "if" <opexpr> "->" <opexpr> "else" <opexpr>
   *    | <operatorexpr>
   */
  private Tree.Expression parseLongExpression(boolean isStatement) throws SyntaxError {
    if (at(Type.FN)) {
      expect(Type.FN);
      List<Parameter> params = new ArrayList<Parameter>();
      String methodName = parseParameters("()", params, false);
      Tree.Expression body = parseFunctionBody(false);
      return Tree.Lambda.create(source, methodName, params, body, "fn");
    } else if (at(Type.IF)) {
      expect(Type.IF);
      Tree.Expression cond = parseLogicalExpression();
      expect(Type.THEN);
      Tree.Expression thenPart = parseLongExpression(isStatement);
      checkSemicolon(isStatement);
      Tree.Expression elsePart;
      if (at(Type.ELSE)) {
        expect(Type.ELSE);
        elsePart = parseLongExpression(isStatement);
        checkSemicolon(isStatement);
      } else {
        elsePart = new Tree.Singleton(Tree.Singleton.Type.NULL);
      }
      return Tree.If.create(source ,cond, thenPart, elsePart);
    } else if (at(Type.FOR)) {
      expect(Type.FOR);
      expect(Type.LPAREN);
      List<Parameter> params = new ArrayList<Parameter>();
      parseNakedParameters(params);
      expect(Type.COLON);
      Tree.Expression coll = parseExpression(false);
      expect(Type.RPAREN);
      Tree.Expression body = parseStatementEnd(isStatement);
      checkSemicolon(isStatement);
      return new Tree.Call("for", Arrays.<Tree.Expression>asList(
          coll,
          Lambda.create(
              source,
              params,
              body,
              "for")
          ));
    } else if (at(Type.WITH_ESCAPE)) {
      expect(Type.WITH_ESCAPE);
      expect(Type.LPAREN);
      String name = expect(Type.IDENT);
      expect(Type.RPAREN);
      Tree.Expression value = parseFunctionBody(isStatement);
      return Tree.WithEscape.create(source, name, value);
    } else {
      Tree.Expression result = parseLogicalExpression();
      checkSemicolon(isStatement);
      return result;
    }
  }

  private Tree.Expression parseStatementEnd(boolean isStatement) throws SyntaxError {
    return parseLongExpression(isStatement);
  }

  /**
   * <expr>
   *   -> <funexpr>
   */
  private Tree.Expression parseExpression(boolean isStatement) throws SyntaxError {
    return parseLongExpression(isStatement);
  }

  private Tree.New.Field parseNewField() throws SyntaxError {
    String name = "." + expect(Type.IDENT);
    List<Parameter> params;
    Tree.Expression body;
    boolean hasEagerValue;
    if (at(Type.COLON_EQ)) {
      expect(Type.COLON_EQ);
      body = parseExpression(false);
      params = Collections.<Parameter>emptyList();
      hasEagerValue = true;
    } else if (at(Type.ARROW)) {
      params = new ArrayList<Parameter>();
      body = parseFunctionBody(false);
      hasEagerValue = false;
    } else {
      throw currentSyntaxError();
    }
    return new Tree.New.Field(params, name, body, hasEagerValue);
  }

  private Tree.Expression parseNewExpression() throws SyntaxError {
    assert at(Type.NEW);
    expect(Type.NEW);
    List<Tree.New.Field> fields = new ArrayList<Tree.New.Field>();
    List<String> protocols = new ArrayList<String>();
    if (!at(Type.LBRACE)) {
      String first = expect(Type.IDENT);
      protocols.add(first);
      while (at(Type.COMMA)) {
        expect(Type.COMMA);
        String next = expect(Type.IDENT);
        protocols.add(next);
      }
    }
    if (at(Type.LBRACE)) {
      expect(Type.LBRACE);
      if (!at(Type.RBRACE)) {
        Tree.New.Field first = parseNewField();
        fields.add(first);
        while (at(Type.COMMA)) {
          expect(Type.COMMA);
          Tree.New.Field next = parseNewField();
          fields.add(next);
        }
      }
      consumeRightBrace();
      StringBuilder buf = new StringBuilder();
      boolean first = true;
      for (String protocol : protocols) {
        if (first) first = false;
        else buf.append("&");
        buf.append(protocol);
      }
      return new Tree.New(source, fields, protocols, buf.toString());
    }
    assert protocols.size() == 1;
    List<Tree.Expression> args = new ArrayList<Tree.Expression>();
    args.add(new Tree.Identifier(protocols.get(0)));
    if (at(Type.LPAREN)) {
      expect(Type.LPAREN);
      args.addAll(parseArguments(Type.RPAREN));
      expect(Type.RPAREN);
    }
    if (at(Type.LBRACK)) {
      expect(Type.LBRACK);
      args.add(parsePrimitiveCollectionContents(Type.RBRACK));
      expect(Type.RBRACK);
    }
    return new Tree.Call("new", args);
  }

  private Tree.Expression parsePrimitiveCollection() throws SyntaxError {
    assert at(Type.HASH);
    expect(Type.HASH);
    expect(Type.LBRACK);
    Tree.Expression result = parsePrimitiveCollectionContents(Type.RBRACK);
    expect(Type.RBRACK);
    return result;
  }

  private Tree.Expression parsePrimitiveCollectionContents(Type end) throws SyntaxError {
    List<Tree.Expression> values = new ArrayList<Tree.Expression>();
    if (!at(Type.RBRACK)) {
      Tree.Expression first = parseExpression(false);
      values.add(first);
      while (at(Type.COMMA)) {
        expect(Type.COMMA);
        Tree.Expression next = parseExpression(false);
        values.add(next);
      }
    }
    return new Tree.Collection(values);
  }

  private int parseNumber(String str) {
    int index = str.indexOf('r');
    int radix;
    String body;
    if (index == -1) {
      radix = 10;
      body = str;
    } else {
      radix = Integer.parseInt(str.substring(0, index));
      body = str.substring(index + 1);
    }
    return (int) Long.parseLong(body, radix);
  }

  private static Tree.Expression toString(Tree.Expression expr) {
    return new Tree.Call("to_string", Arrays.asList(expr));
  }

  private static Tree.Expression add(Tree.Expression a, Tree.Expression b) {
    return new Tree.Call("+", Arrays.asList(a, b));
  }

  private Tree.Expression parseInterpolExpression() throws SyntaxError {
    String start = expect(Type.INTERPOL_START);
    Tree.Expression first = parseExpression(false);
    Tree.Expression result = new Tree.Text(start);
    result = add(result, toString(first));
    while (!at(Type.INTERPOL_END)) {
      String part = expect(Type.INTERPOL_PART);
      result = add(result, new Tree.Text(part));
      Tree.Expression next = parseExpression(false);
      result = add(result, toString(next));
    }
    String last = expect(Type.INTERPOL_END);
    result = add(result, new Tree.Text(last));
    return result;
  }

  /**
   * <atomic>
   *   -> <ident>
   *   -> <number>
   *   -> "(" <expr> ")"
   *   -> <block>
   *   -> "null" | "true" | "false"
   *   -> <new>
   *   -> "native" "(" <id> ")"
   */
  private Tree.Expression parseAtomicExpression() throws SyntaxError {
    Token current = getCurrent();
    switch (current.getType()) {
    case IDENT: {
      String value = expect(Type.IDENT);
      return new Tree.Identifier(value);
    }
    case NUMBER: {
      String str = expect(Type.NUMBER);
      int value = parseNumber(str);
      return new Tree.Number(value);
    }
    case NEW:
      return parseNewExpression();
    case LPAREN: {
      expect(Type.LPAREN);
      Tree.Expression result = parseExpression(false);
      expect(Type.RPAREN);
      return result;
    }
    case INTERPOL_START: {
      return parseInterpolExpression();
    }
    case LBRACE: {
      Tree.Expression result = parseBlock();
      hasTransientSemicolon = true;
      return result;
    }
    case NULL:
      expect(Type.NULL);
      return new Tree.Singleton(Tree.Singleton.Type.NULL);
    case TRUE:
      expect(Type.TRUE);
      return new Tree.Singleton(Tree.Singleton.Type.TRUE);
    case FALSE:
      expect(Type.FALSE);
      return new Tree.Singleton(Tree.Singleton.Type.FALSE);
    case STRING:
      String value = expect(Type.STRING);
      return new Tree.Text(value);
    case INTERNAL:
      expect(Type.INTERNAL);
      String name = expect(Type.STRING);
      return new Tree.Internal(name);
    case HASH:
      return parsePrimitiveCollection();
    default:
      throw currentSyntaxError();
    }
  }

  public static Tree.Unit parse(Source source, List<Token> tokens) throws SyntaxError {
    return new Parser(source, tokens).parseUnit();
  }

}
