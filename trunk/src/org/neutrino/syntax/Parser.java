package org.neutrino.syntax;

import java.util.ArrayList;
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
    List<Tree.Definition> defs = new ArrayList<Tree.Definition>();
    while (hasMore()) {
      Tree.Definition def = parseDefinition();
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
   */
  private Tree.Definition parseDefinition() throws SyntaxError {
    List<String> annots = parseAnnotations();
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
   * <funexpr>
   *   -> "fn" <args> <funbody>
   *    | <atomic>
   */
  private Tree.Expression parseFunctionExpression() throws SyntaxError {
    if (at(Type.FN)) {
      expect(Type.FN);
      List<String> params = parseParameters();
      Tree.Expression body = parseFunctionBody(false);
      return new Tree.Lambda(params, body);
    } else {
      return parseAtomicExpression();
    }
  }

  /**
   * <expr>
   *   -> <atomic>
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
    default:
      throw new SyntaxError(current);
    }
  }

  public static Tree.Unit parse(List<Token> tokens) throws SyntaxError {
    return new Parser(tokens).parseUnit();
  }

}
