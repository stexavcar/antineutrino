package org.neutrino.syntax;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.neutrino.syntax.Token.Type;

/**
 * Plain vanilla recursive descent parser for the neutrino syntax.
 * Main entry point is {@link #parse(List)}.
 *
 * @author christian.plesner.hansen@gmail.com (Christian Plesner Hansen)
 */
public class Parser {

  private int cursor = 0;
  private final List<Token> tokens;

  private Parser(List<Token> tokens) {
    this.tokens = tokens;
  }

  private Token getCurrent() {
    return tokens.get(cursor);
  }

  private void advance() {
    cursor++;
  }

  private boolean hasMore() {
    return cursor < tokens.size();
  }

  private Tree.Unit parseUnit() throws SyntaxError {
    List<Tree.Declaration> defs = new ArrayList<Tree.Declaration>();
    while (hasMore()) {
      Tree.Declaration def = parseDefinition();
      defs.add(def);
    }
    return new Tree.Unit(defs);
  }

  private String expect(Type type) throws SyntaxError {
    if (!hasMore())
      throw new SyntaxError(new Token(Type.ERROR, "<eof>", 0, 0));
    Token current = getCurrent();
    if (!current.getType().equals(type))
      throw new SyntaxError(current);
    String value = current.getValue();
    advance();
    return value;
  }

  private boolean at(Type type) {
    return getCurrent().getType() == type;
  }

  private boolean atOperator() {
    return getCurrent().getType().isOperator();
  }

  /**
   * <statement>
   */
  private Tree.Expression parseStatement() throws SyntaxError {
    if (at(Type.DEF)) {
      expect(Type.DEF);
      String name = expect(Type.IDENT);
      expect(Type.COLON_EQ);
      Tree.Expression value = parseExpression();
      Tree.Expression body;
      if (at(Type.SEMI)) {
        expect(Type.SEMI);
        if (hasMore() && !at(Type.RBRACE)) {
          body = parseStatements();
        } else {
          body = new Tree.Singleton(Tree.Singleton.Type.NULL);
        }
      } else {
        body = new Tree.Singleton(Tree.Singleton.Type.NULL);
      }
      return new Tree.LocalDefinition(name, value, body);
    } else {
      return parseExpression();
    }
  }

  /**
   * <statements>
   *   <statement> +: ";"+ ";"?
   */
  private Tree.Expression parseStatements() throws SyntaxError {
    Tree.Expression expr = parseStatement();
    if (at(Type.SEMI)) {
      expect(Type.SEMI);
      if (hasMore() && !at(Type.RBRACE)) {
        List<Tree.Expression> exprs = new ArrayList<Tree.Expression>();
        exprs.add(expr);
        while (hasMore() && !at(Type.RBRACE)) {
          Tree.Expression next = parseStatement();
          exprs.add(next);
          if (at(Type.SEMI)) {
            expect(Type.SEMI);
          } else {
            break;
          }
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
   * @return
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
      return body;
    } else {
      throw new SyntaxError(getCurrent());
    }
  }

  /**
   * <annots>
   *   -> ("@" <ident>)*
   */
  private List<String> parseAnnotations() throws SyntaxError {
    List<String> annots = new ArrayList<String>();
    while (at(Type.AT)) {
      expect(Type.AT);
      String name = expect(Type.IDENT);
      annots.add(name);
    }
    return annots;
  }

  /**
   * <def>
   *   -> <annots> "def" <ident> ":=" <expr> ";"
   *   -> <annots> "def" <ident> <params> <funbody>
   *   -> <annots> "protocol" <ident> ";"
   */
  private Tree.Declaration parseDefinition() throws SyntaxError {
    List<String> annots = parseAnnotations();
    if (at(Type.DEF)) {
      expect(Type.DEF);
      String name = expect(Type.IDENT);
      if (at(Type.COLON_EQ)) {
        expect(Type.COLON_EQ);
        Tree.Expression value = parseExpression();
        expect(Type.SEMI);
        return new Tree.Definition(annots, name, value);
      } else if (at(Type.ARROW) || at(Type.LPAREN) || at(Type.LBRACE)) {
        List<String> params = parseParameters();
        Tree.Expression body = parseFunctionBody(true);
        Tree.Lambda lambda = new Tree.Lambda(params, body);
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

  private List<String> parseParameters() throws SyntaxError {
    if (at(Type.LPAREN)) {
      expect(Type.LPAREN);
      List<String> result = new ArrayList<String>();
      if (!at(Type.RPAREN)) {
        while (true) {
          String name = expect(Type.IDENT);
          result.add(name);
          if (at(Type.COMMA)) {
            expect(Type.COMMA);
            continue;
          } else {
            break;
          }
        }
      }
      expect(Type.RPAREN);
      return result;
    } else {
      return Collections.emptyList();
    }
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
    if (at(Type.DOT)) {
      expect(Type.DOT);
      method = expect(Type.IDENT);
    } else if (at(Type.LPAREN)) {
      method = "()";
    } else {
      throw new SyntaxError(getCurrent());
    }
    List<Tree.Expression> args = new ArrayList<Tree.Expression>();
    args.add(recv);
    if (at(Type.LPAREN)) {
      expect(Type.LPAREN);
      args.addAll(parseArguments(Type.RPAREN));
      expect(Type.RPAREN);
    }
    return new Tree.Call(method, args);
  }

  /**
   * <callexpr>
   *   -> <atomic> (<callexpr-tail>)*
   */
  private Tree.Expression parseCallExpression() throws SyntaxError {
    Tree.Expression current = parseAtomicExpression();
    while (at(Type.DOT) || at(Type.LPAREN)) {
      current = parseCallExpressionTail(current);
    }
    return current;
  }

  private Tree.Expression parseOperatorExpression() throws SyntaxError {
    Tree.Expression current = parseCallExpression();
    while (atOperator()) {
      String op = getCurrent().getValue();
      advance();
      Tree.Expression next = parseCallExpression();
      current = new Tree.Call(op, Arrays.asList(current, next));
    }
    return current;
  }

  /**
   * <funexpr>
   *   -> "fn" <args> <funbody>
   *    | <operatorexpr>
   */
  private Tree.Expression parseFunctionExpression() throws SyntaxError {
    if (at(Type.FN)) {
      expect(Type.FN);
      List<String> params = parseParameters();
      Tree.Expression body = parseFunctionBody(false);
      return new Tree.Lambda(params, body);
    } else {
      return parseOperatorExpression();
    }
  }

  /**
   * <expr>
   *   -> <funexpr>
   */
  private Tree.Expression parseExpression() throws SyntaxError {
    return parseFunctionExpression();
  }

  /**
   * <atomic>
   *   -> <ident>
   *   -> <number>
   *   -> "(" <expr> ")"
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
      int value = Integer.parseInt(str);
      return new Tree.Number(value);
    }
    case LPAREN: {
      expect(Type.LPAREN);
      Tree.Expression result = parseExpression();
      expect(Type.RPAREN);
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
    default:
      throw new SyntaxError(current);
    }
  }

  public static Tree.Unit parse(List<Token> tokens) throws SyntaxError {
    return new Parser(tokens).parseUnit();
  }

}
