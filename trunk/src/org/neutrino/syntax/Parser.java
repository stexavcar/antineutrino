package org.neutrino.syntax;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.neutrino.pib.Parameter;
import org.neutrino.runtime.RValue;
import org.neutrino.syntax.Token.Type;
import org.neutrino.syntax.Tree.If;

/**
 * Plain vanilla recursive descent parser for the neutrino syntax.
 * Main entry point is {@link #parse(List)}.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Parser {

  private int cursor = 0;
  private final List<Token> tokens;
  private boolean hasTransientSemicolon = false;

  private Parser(List<Token> tokens) {
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
      Tree.Declaration def = parseDeclaration();
      defs.add(def);
    }
    return new Tree.Unit(defs);
  }

  private String expect(Type type) throws SyntaxError {
    if (!hasMore())
      throw new SyntaxError(new Token(Type.ERROR, "<eof>", 0, 0), type);
    Token current = getCurrent();
    if (!current.getType().equals(type))
      throw new SyntaxError(current, type);
    String value = current.getValue();
    advance();
    return value;
  }

  private boolean at(Type type) {
    return getCurrent().getType() == type;
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
    if (atDefinitionStart()) {
      boolean isReference = consumeDefinitionStart();
      String name = expect(Type.IDENT);
      Tree.Expression value;
      if (at(Type.COLON_EQ)) {
        expect(Type.COLON_EQ);
        value = parseExpression();
      } else if (atFunctionDefinitionMarker()) {
        List<Parameter> params = new ArrayList<Parameter>();
        String methodName = parseParameters("()", params);
        Tree.Expression body = parseFunctionBody(false);
        value = Tree.Lambda.create(methodName, params, body);
      } else {
        throw new SyntaxError(getCurrent());
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
      return new Tree.LocalDefinition(name, value, body, isReference);
    } else {
      return parseExpression();
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

  /**
   * <statements>
   *   <statement> +: ";"+ ";"?
   */
  private Tree.Expression parseStatements() throws SyntaxError {
    Tree.Expression expr = parseStatement();
    if (consumeSemicolon()) {
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
    } else {
      return expr;
    }
  }

  /**
   * <block>
   *   -> "{" <statement> ... "}"
   */
  private Tree.Expression parseBlock() throws SyntaxError {
    expect(Type.LBRACE);
    if (at(Type.RBRACE)) {
      expect(Type.RBRACE);
      return new Tree.Singleton(Tree.Singleton.Type.NULL);
    } else {
      Tree.Expression result = parseStatements();
      expect(Type.RBRACE);
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
      Tree.Expression body = parseExpression();
      if (isStatement)
        expect(Type.SEMI);
      return body;
    } else if (at(Type.LBRACE)) {
      Tree.Expression body = parseBlock();
      hasTransientSemicolon = true;
      return body;
    } else if (isStatement && at(Type.SEMI)) {
      expect(Type.SEMI);
      return new Tree.Singleton(Tree.Singleton.Type.NULL);
    } else {
      throw new SyntaxError(getCurrent());
    }
  }

  /**
   * <annots>
   *   -> ("@" <ident>)*
   */
  private List<Annotation> parseAnnotations() throws SyntaxError {
    List<Annotation> annots = new ArrayList<Annotation>();
    while (at(Type.AT)) {
      expect(Type.AT);
      String name = expect(Type.IDENT);
      List<RValue> args = Collections.emptyList();
      if (at(Type.LPAREN)) {
        expect(Type.LPAREN);
        if (!at(Type.RPAREN)) {
          args = new ArrayList<RValue>();
          Tree.Expression first = parseExpression();
          args.add(first.toValue());
          while (at(Type.COMMA)) {
            expect(Type.COMMA);
            Tree.Expression next = parseExpression();
            args.add(next.toValue());
          }
        }
        expect(Type.RPAREN);
      }
      annots.add(new Annotation(name, args));
    }
    return annots;
  }

  private String expectName(boolean allowEmpty) throws SyntaxError {
    if (atOperator()) {
      return expectOperator();
    } else if (at(Type.IDENT) || !allowEmpty) {
      return expect(Type.IDENT);
    } else {
      return null;
    }
  }

  private boolean atFunctionDefinitionMarker() {
    return at(Type.ARROW) || at(Type.LPAREN) || at(Type.LBRACE) || at(Type.LBRACK);
  }

  /**
   * <def>
   *   -> <annots> "def" <ident> ":=" <expr> ";"
   *   -> <annots> "def" <ident> "::" <methodname> <params> <funbody>
   *   -> <annots> "def" <ident> <params> <funbody>
   *   -> <annots> "protocol" <ident> ";"
   */
  private Tree.Declaration parseDeclaration() throws SyntaxError {
    List<Annotation> annots = parseAnnotations();
    if (at(Type.DEF)) {
      expect(Type.DEF);
      String name = expect(Type.IDENT);
      if (at(Type.COLON_EQ)) {
        expect(Type.COLON_EQ);
        Tree.Expression value = parseExpression();
        expect(Type.SEMI);
        return new Tree.Definition(annots, name, value);
      } else if (at(Type.COOLON)) {
        expect(Type.COOLON);
        String method = expectName(true);
        List<Parameter> argParams = new ArrayList<Parameter>();
        method = parseParameters(method, argParams);
        Tree.Expression body = parseFunctionBody(true);
        List<Parameter> params = new ArrayList<Parameter>();
        params.add(new Parameter("this", name));
        params.addAll(argParams);
        return new Tree.Method(annots, method, params, body);
      } else if (at(Type.IS)) {
        expect(Type.IS);
        String parent = expect(Type.IDENT);
        expect(Type.SEMI);
        return new Tree.Inheritance(annots, name, parent);
      } else if (atFunctionDefinitionMarker()) {
        List<Parameter> params = new ArrayList<Parameter>();
        String methodName = parseParameters("()", params);
        Tree.Expression body = parseFunctionBody(true);
        Tree.Expression lambda = Tree.Lambda.create(methodName, params, body);
        return new Tree.Definition(annots, name, lambda);
      } else {
        throw new SyntaxError(getCurrent());
      }
    } else {
      expect(Type.PROTOCOL);
      String name = expect(Type.IDENT);
      expect(Type.SEMI);
      return new Tree.Protocol(annots, name);
    }
  }

  private Parameter parseParameter() throws SyntaxError {
    String name = expect(Type.IDENT);
    String type = "Object";
    if (at(Type.COLON)) {
      expect(Type.COLON);
      type = expect(Type.IDENT);
    }
    return new Parameter(name, type);
  }

  private String parseParameters(String name, List<Parameter> params) throws SyntaxError {
    String methodName = name;
    Token.Type begin = Type.LPAREN;
    Token.Type end = Type.RPAREN;
    if (at(Type.LBRACK)) {
      begin = Type.LBRACK;
      end = Type.RBRACK;
      methodName = "[]";
    }
    if (at(begin)) {
      expect(begin);
      if (!at(end)) {
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
      expect(end);
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
      Tree.Expression first = parseExpression();
      result.add(first);
    }
    while (at(Type.COMMA)) {
      expect(Type.COMMA);
      Tree.Expression next = parseExpression();
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
      method = expect(Type.IDENT);
    } else if (at(Type.LPAREN)) {
      method = "()";
    } else if (at(Type.LBRACK)) {
      method = "[]";
      argDelimStart = Type.LBRACK;
      argDelimEnd = Type.RBRACK;
    } else {
      throw new SyntaxError(getCurrent());
    }
    List<Tree.Expression> args = new ArrayList<Tree.Expression>();
    args.add(recv);
    if (at(argDelimStart)) {
      expect(argDelimStart);
      args.addAll(parseArguments(argDelimEnd));
      expect(argDelimEnd);
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
      current = If.create(current, next,
          new Tree.Singleton(Tree.Singleton.Type.FALSE));
    }
    return current;
  }

  private Tree.Expression parseOrExpression() throws SyntaxError {
    Tree.Expression current = parseAndExpression();
    while (at(Type.OR)) {
      expect(Type.OR);
      Tree.Expression next = parseAndExpression();
      current = If.create(current,
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
  private Tree.Expression parseLongExpression() throws SyntaxError {
    if (at(Type.FN)) {
      expect(Type.FN);
      List<Parameter> params = new ArrayList<Parameter>();
      String methodName = parseParameters("()", params);
      Tree.Expression body = parseFunctionBody(false);
      return Tree.Lambda.create(methodName, params, body);
    } else if (at(Type.IF)) {
      expect(Type.IF);
      Tree.Expression cond = parseLogicalExpression();
      expect(Type.THEN);
      Tree.Expression thenPart = parseLongExpression();
      Tree.Expression elsePart;
      if (at(Type.ELSE)) {
        expect(Type.ELSE);
        elsePart = parseLongExpression();
      } else {
        elsePart = new Tree.Singleton(Tree.Singleton.Type.NULL);
      }
      return Tree.If.create(cond, thenPart, elsePart);
    } else if (at(Type.WITH_1CC)) {
      expect(Type.WITH_1CC);
      String name = expect(Type.IDENT);
      expect(Type.ARROW);
      Tree.Expression value = parseLongExpression();
      return Tree.With1Cc.create(name, value);
    } else {
      return parseLogicalExpression();
    }
  }

  /**
   * <expr>
   *   -> <funexpr>
   */
  private Tree.Expression parseExpression() throws SyntaxError {
    return parseLongExpression();
  }

  private Tree.New.Field parseNewField() throws SyntaxError {
    String name = expect(Type.IDENT);
    List<Parameter> params;
    Tree.Expression body;
    boolean hasEagerValue;
    if (at(Type.COLON_EQ)) {
      expect(Type.COLON_EQ);
      body = parseExpression();
      params = Collections.<Parameter>emptyList();
      hasEagerValue = true;
    } else if (at(Type.ARROW)) {
      params = new ArrayList<Parameter>();
      body = parseFunctionBody(false);
      hasEagerValue = false;
    } else {
      throw new SyntaxError(getCurrent());
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
      expect(Type.RBRACE);
    }
    return new Tree.New(fields, protocols, protocols.toString());
  }

  private Tree.Expression parsePrimitiveCollection() throws SyntaxError {
    assert at(Type.HASH);
    expect(Type.HASH);
    expect(Type.LBRACK);
    List<Tree.Expression> values = new ArrayList<Tree.Expression>();
    if (!at(Type.RBRACK)) {
      Tree.Expression first = parseExpression();
      values.add(first);
      while (at(Type.COMMA)) {
        expect(Type.COMMA);
        Tree.Expression next = parseExpression();
        values.add(next);
      }
    }
    expect(Type.RBRACK);
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
      Tree.Expression result = parseExpression();
      expect(Type.RPAREN);
      return result;
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
      throw new SyntaxError(current);
    }
  }

  public static Tree.Unit parse(List<Token> tokens) throws SyntaxError {
    return new Parser(tokens).parseUnit();
  }

}