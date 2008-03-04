#!/usr/bin/python
# -*- coding: utf-8 -*-

# A compiler that produces binaries to be executed by the neutrino
# runtime.  This is not the "real" compiler but is only used
# when bootstrapping the runtime.

from optparse import OptionParser, Option
from os import path
from buildutils import KEYWORDS
import re, string, struct, codecs

INTERNAL = 'internal'
NATIVE = 'native'
STATIC = 'static'

# -------------------
# --- T o k e n s ---
# -------------------

class Token:
  def is_eof(self): return False
  def is_keyword(self, name): return False
  def is_ident(self): return False
  def is_delimiter(self, name): return False
  def is_number(self): return False
  def is_string(self): return False
  def is_operator(self, name = None): return False
  def is_doc(self): return False

class Keyword(Token):
  def __init__(self, value):
    self.value = value
  def is_keyword(self, name):
    return name == self.value
  def __str__(self):
    return self.value

class Ident(Token):
  def __init__(self, name):
    self.name = name
    self.value = name
  def is_ident(self):
    return True

class Operator(Token):
  def __init__(self, name):
    self.name = name
  def is_operator(self):
    return True
  def is_operator(self, name = None):
    return (not name) or (self.name == name)

class NumberToken(Token):
  def __init__(self, value):
    self.value = value
  def is_number(self):
    return True

class StringToken(Token):
  def __init__(self, terms):
    self.terms = terms
  def is_string(self):
    return True

class DocToken(Token):
  def __init__(self, value):
    self.value = value
  def is_doc(self):
    return True

class EOS(Token):
  def is_eof(self):
    return True

class Delimiter(Token):
  def __init__(self, value):
    self.value = value
  def is_delimiter(self, name):
    return name == self.value
  def __str__(self):
    return self.value

RESERVED = [
  '->', '~'
]


# ---------------------
# --- S c a n n e r ---
# ---------------------

def is_space(char):
  return char.isspace()

def is_ident_start(char):
  return char.isalpha() or (char == '_')

def is_digit(char):
  return char.isdigit()

def is_ident_part(char):
  return char.isdigit() or is_ident_start(char)

def is_operator(char):
  return char == '+' or char == '-' or char == '>' or char == '*'    \
      or char == '/' or char == '%' or char == '=' or char == '<'    \
      or char == '~'                                                 \
      or is_circumfix_operator(char)

def is_circumfix_operator(char):
  return char == '|' or char == u'‖'

def circumfix_match(char):
  if char == '|': return '|'
  elif char == u'‖': return u'‖'
  else: raise "Error"

class Scanner:

  def __init__(self, input):
    self.input = input
    self.cursor = 0
    self.skip_whitespace()
    self.parser = None

  def set_parser(self, parser):
    self.parser = parser

  def current(self):
    if self.has_more(): return self.input[self.cursor]
    else: return None

  def next(self):
    if self.has_next(): return self.input[self.cursor + 1]
    else: return None

  def advance(self, dist = 1):
    if self.has_more(): self.cursor += dist
    else: raise "OutOfBounds"

  def has_more(self):
    return self.cursor < len(self.input)

  def has_next(self):
    return self.cursor + 1 < len(self.input)

  def scan_ident(self):
    value = ''
    while self.has_more() and is_ident_part(self.current()):
      value += self.current()
      self.advance()
    if value in KEYWORDS: return Keyword(value)
    else: return Ident(value)

  def scan_operator(self):
    value = ''
    while self.has_more() and is_operator(self.current()):
      value += self.current()
      self.advance()
    if value in RESERVED: return Delimiter(value)
    else: return Operator(value)

  def scan_number(self):
    value = 0
    while self.has_more() and is_digit(self.current()):
      value = 10 * value + (ord(self.current()) - ord('0'))
      self.advance()
    return NumberToken(value)

  def scan_string(self):
    terms = []
    self.advance()
    term = ''
    while self.has_more() and self.current() != '"':
      if self.current() =='$' and self.next() == '{':
        if len(term) > 0:
          terms.append(term)
          term = ''
        self.advance(2)
        self.parser.advance()
        expr = self.parser.parse_expression(False)
        terms.append(expr)
      else:
        term += self.current()
        self.advance()
    if self.has_more(): self.advance()
    if len(term) > 0:
      terms.append(term)
    return StringToken(terms)

  def scan_documentation(self):
    value = ''
    while self.has_more() and self.current() == '#':
      while self.has_more() and self.current() != '\n':
        value += self.current()
        self.advance()
      self.skip_whitespace()
    return DocToken(value)

  def scan_delimiter(self):
    c = self.current()
    self.advance()
    if c == '(':
      return Delimiter('(')
    elif c == ')':
      return Delimiter(')')
    elif c == ';':
      return Delimiter(';')
    elif c == ':':
      if self.current() == '=':
        self.advance()
        return Delimiter(':=')
      else:
        return Delimiter(':')
    elif c == ',':
      return Delimiter(',')
    elif c == '.':
      return Delimiter('.')
    elif c == '{':
      return Delimiter('{')
    elif c == '}':
      return Delimiter('}')
    elif c == '[':
      return Delimiter('[')
    elif c == ']':
      return Delimiter(']')
    elif c == u'·':
      return Delimiter(u'·')
    elif c == '-':
      if not self.has_more(): return Delimiter('-')
      c = self.current()
      if c == '>':
        self.advance()
        return Delimiter('->')
      else:
        return Delimiter('-')
    else:
      raise "Unexpected character " + str(c)

  def get_next_token(self):
    if is_ident_start(self.current()):
      result = self.scan_ident()
    elif is_digit(self.current()):
      result = self.scan_number()
    elif self.current() == '"':
      result = self.scan_string();
    elif is_operator(self.current()):
      result = self.scan_operator()
    elif self.current() == '#':
      result = self.scan_documentation()
    else:
      result = self.scan_delimiter()
    self.skip_whitespace()
    return result

  def skip_whitespace(self):
    keep_going = True
    while keep_going:
      keep_going = False
      while self.has_more() and is_space(self.current()):
        self.advance()
      if self.current() == '/' and self.next() == '/':
        keep_going = True
        while self.has_more() and self.current() != '\n':
          self.advance()
      elif self.current() == '(' and self.next() == '*':
        keep_going = True
        while self.has_more() and (self.current() != '*' or self.next() != ')'):
          self.advance()
        if self.has_more() and self.has_next():
          self.advance()
          self.advance()

# -------------------
# --- P a r s e r ---
# -------------------

class QuoteLevel:
  def __init__(self, parent):
    self.parent = parent
    self.unquotes = []
  def register_unquoted(self, expr):
    index = len(self.unquotes)
    self.unquotes.append(expr)
    return index

def new_sequence(exprs):
  if len(exprs) == 0: return Void()
  elif len(exprs) == 1: return exprs[0]
  else: return Sequence(exprs)

class Parser:

  def __init__(self, scanner):
    self.scanner = scanner
    self.scope = GlobalScope()
    self.quote_level = None
    self.current = None
    self.advance()

  # Advances the cursor one step
  def advance(self):
    if self.scanner.has_more():
      self.current = self.scanner.get_next_token()
    else:
      self.current = EOS()

  # Returns the current token, or <eof> when reaching the end of the
  # input
  def token(self):
    return self.current

  def parse_error(self):
    raise "SyntaxError: '" + str(self.token()) + "'"

  def expect_keyword(self, expected):
    if not self.token().is_keyword(expected):
      print "Expected " + str(expected) + ", found " + self.token().value
      self.parse_error()
    self.advance()

  def expect_delimiter(self, expected):
    if not self.token().is_delimiter(expected):
      print "Expected " + str(expected) + ", found " + str(self.token())
      self.parse_error()
    self.advance()

  # Ensures that the current token is an identifier and then advances
  # the cursor.  If the current token is not correct, an error is
  # reported.
  def expect_ident(self):
    if not self.token().is_ident():
      print "Expected identifier, found " + str(self.token())
      self.parse_error()
    name = self.token().name
    self.advance()
    return name

  def expect_operator(self, op = None):
    if not self.token().is_operator(op):
      print "Expected operator, found " + str(self.token())
      self.parse_error()
    name = self.token().name
    self.advance()
    return name

  def expect_circumfix_operator(self):
    if not self.token().is_operator():
      print "Expected operator, found " + str(self.token())
      self.parse_error()
    name = self.token().name
    if not is_circumfix_operator(name):
      print "Expected circumfix, found " + name
      self.parse_error()
    self.advance()
    return name

  # Ensures that the current token is a number and then advances the
  # cursor.  If the current token is not correct, an error is
  # reported.
  def expect_number(self):
    if not self.token().is_number():
      print "Expected number, found " + str(token)
      self.parse_error()
    value = self.token().value
    self.advance()
    return value

  # <program>
  #   -> <declaration>*
  def parse_program(self):
    decls = []
    while not self.token().is_eof():
      decl = self.parse_declaration()
      decls.append(decl)
    return Program(decls)

  def parse_doc_opt(self):
    if self.token().is_doc():
      value = self.token().value
      self.advance()
      return value
    else:
      return None

  # <declaration>
  #   -> <doc>? <definition>
  #   -> <doc>? <class_declaration>
  def parse_declaration(self):
    doc = self.parse_doc_opt()
    modifiers = self.parse_modifiers()
    if self.token().is_keyword('def'):
      return self.parse_definition(modifiers)
    elif self.token().is_keyword('protocol'):
      return self.parse_protocol_declaration(modifiers)
    else:
      assert False

  def is_modifier(self, token):
    return (token.is_keyword(INTERNAL) or token.is_keyword(NATIVE) or
            token.is_keyword(STATIC))

  def has_body(self, modifiers):
    return (not INTERNAL in modifiers) and (not NATIVE in modifiers)

  # <modifiers>
  #   -> <modifier>*
  def parse_modifiers(self):
    modifiers = { }
    while self.is_modifier(self.token()):
      modifiers[self.token().value] = True
      self.advance()
    return modifiers

  def expect_method_name(self):
    if self.token().is_operator():
      return self.expect_operator();
    elif self.token().is_keyword('new'):
      self.expect_keyword('new')
      return 'new'

  # <member_name>
  #   -> $ident
  #   -> 'operator' $operator
  def parse_member_name(self):
    if self.token().is_ident():
      return self.expect_ident()
    else:
      self.expect_keyword('operator')
      return self.expect_method_name()

  # <protocol_declaration>
  #   -> 'protocol' $ident '{' <member_declaration>* '}'
  def parse_protocol_declaration(self, modifiers):
    self.expect_keyword('protocol')
    name = self.expect_ident()
    if self.token().is_delimiter(':'):
      self.expect_delimiter(':')
      parent = self.expect_ident()
    else:
      parent = None
    self.expect_delimiter('{')
    members = []
    while not self.token().is_delimiter('}'):
      member = self.parse_member_declaration(name)
      members.append(member)
    self.expect_delimiter('}')
    if INTERNAL in modifiers:
      info = BUILTIN_CLASSES[name]
      return BuiltinClass(info, members, parent)
    else:
      return Protocol(name, members, parent)

  def pop_scope(self):
    self.scope = self.scope.parent

  def push_scope(self, symbols):
    self.scope = LocalScope(self.scope, symbols)

  # <member_declaration>
  #   -> <modifiers> 'def' <method_header> <method_body>
  def parse_member_declaration(self, class_name):
    doc = self.parse_doc_opt()
    modifiers = self.parse_modifiers()
    self.expect_keyword('def')
    name = self.parse_member_name()
    params = self.parse_params('(', ')')
    is_static = STATIC in modifiers
    if INTERNAL in modifiers:
      self.expect_delimiter(';')
      key = (class_name, name)
      if not key in BUILTIN_METHODS:
        raise "Unknown built-in method " + str(class_name) + " " + str(name)
      index = BUILTIN_METHODS[key]
      fun = Lambda(params, Return(BuiltinCall(len(params), index)))
    else:
      self.push_scope(params)
      body = self.parse_function_body(True)
      fun = Lambda(params, body)
      self.pop_scope()
    return Method(is_static, name, fun)

  # <definition>
  #   -> 'def' $ident ':=' <expression> ';'
  #   -> 'def' $ident '(' <params> ')' <function-body>
  def parse_definition(self, modifiers):
    self.expect_keyword('def')
    name = self.expect_ident()
    if self.token() == ':=':
      self.expect(':=')
      value = self.parse_expression(False)
      self.expect_delimiter(';')
      return Definition(name, value)
    else:
      params = self.parse_params('(', ')')
      if not self.has_body(modifiers):
        if INTERNAL in modifiers:
          index = BUILTIN_FUNCTIONS[name]
          fun = Lambda(params, Return(BuiltinCall(len(params), index)))
        else:
          assert NATIVE in modifiers
          fun = Lambda(params, Return(ExternalCall(len(params), name)))
        self.expect_delimiter(';')
      else:
        self.push_scope(params)
        body = self.parse_function_body(True)
        fun = Lambda(params, body)
        self.pop_scope()
      return Definition(name, fun)

  # <function-body>
  #   -> '->' <expression> ';'?
  #   -> <block-expression>
  def parse_function_body(self, is_toplevel):
    if self.token().is_delimiter('->'):
      self.expect_delimiter('->')
      value = self.parse_expression(False)
      if is_toplevel: self.expect_delimiter(';')
      return Return(value)
    elif self.token().is_delimiter('{'):
      self.expect_delimiter('{')
      exprs = self.parse_statements('}')
      self.expect_delimiter('}')
      exprs.append(Return(Void()))
      return new_sequence(exprs)

  def parse_param(self):
    if self.token().is_delimiter('~'):
      return self.parse_unquote_expression()
    else:
      name = self.expect_ident()
      if self.token().is_delimiter(':'):
        is_keyword = True
        self.expect_delimiter(':')
      else:
        is_keyword = False
      return Symbol(name)

  # <params>
  #   -> <param> *: ','
  def parse_params(self, start, end):
    params = []
    self.expect_delimiter(start)
    if not self.token().is_delimiter(end):
      params.append(self.parse_param())
      while self.token().is_delimiter(','):
        self.expect_delimiter(',')
        params.append(self.parse_param())
    self.expect_delimiter(end)
    return params

  def parse_statements(self, end):
    assert end
    stmts = []
    while not self.token().is_delimiter(end):
      stmt = self.parse_control_expression(True, end)
      stmts.append(stmt)
    return stmts

  # <expression>
  #   -> <control_expression>
  def parse_expression(self, is_toplevel):
    return self.parse_control_expression(is_toplevel)

  # <arguments>
  #   -> <expression> *: ','
  def parse_arguments(self, start, end):
    self.expect_delimiter(start)
    args = []
    keywords = { }
    def parse_argument():
      expr = self.parse_expression(False)
      if self.token().is_delimiter(':') and expr.is_identifier():
        self.expect_delimiter(':')
        keywords[expr.name] = len(args)
        expr = self.parse_expression(False)
      args.append(expr)
    if not self.token().is_delimiter(end):
      parse_argument()
    while self.token().is_delimiter(','):
      self.advance()
      parse_argument()
    self.expect_delimiter(end)
    return Arguments(args, keywords)

  def parse_expressions(self, start, end):
    self.expect_delimiter(start)
    args = []
    if not self.token().is_delimiter(end):
      expr = self.parse_expression(False)
      args.append(expr)
    while self.token().is_delimiter(','):
      self.advance()
      expr = self.parse_expression(False)
      args.append(expr)
    self.expect_delimiter(end)
    return args

  def parse_local_definition(self, is_toplevel, end):
    self.expect_keyword('def')
    name = Symbol(self.expect_ident())
    self.expect_delimiter(':=')
    value = self.parse_expression(False)
    if self.token().is_keyword('in'):
      self.expect_keyword('in')
      self.push_scope([ name ])
      body = self.parse_expression(is_toplevel)
      self.pop_scope()
      return LocalDefinition(name, value, body)
    elif is_toplevel:
      self.expect_delimiter(';')
      self.push_scope([ name ])
      stmts = self.parse_statements(end)
      self.pop_scope()
      return LocalDefinition(name, value, new_sequence(stmts))
    else:
      assert False

  def parse_do_expression(self, is_toplevel, end):
    self.expect_keyword('do')
    value = self.parse_expression(is_toplevel)
    clauses = [ ]
    while self.token().is_keyword('on'):
      self.expect_keyword('on')
      name = self.expect_ident()
      params = self.parse_params('(', ')')
      self.expect_delimiter('->')
      body = self.parse_expression(is_toplevel)
      clauses.append(OnClause(name, Lambda(params, Return(body))))
    return DoOnExpression(value, clauses)

  # <control_expression>
  #   -> 'return' <expression> ';'?
  #   -> 'raise' $name <arguments> ';'?
  #   -> <do-expression>
  #   -> <conditional-expression>
  #   -> <operator-expression> ';'?
  def parse_control_expression(self, is_toplevel, end = None):
    if self.token().is_keyword('return'):
      self.expect_keyword('return')
      value = self.parse_expression(False)
      if is_toplevel: self.expect_delimiter(';')
      return Return(value)
    elif self.token().is_keyword('yield'):
      self.expect_keyword('yield')
      value = self.parse_expression(False)
      if is_toplevel: self.expect_delimiter(';')
      return Yield(value)
    elif self.token().is_keyword('raise'):
      self.expect_keyword('raise')
      name = self.expect_ident()
      args = self.parse_arguments('(', ')')
      if is_toplevel: self.expect_delimiter(';')
      return Raise(name, args)
    elif self.token().is_keyword('if'):
      return self.parse_conditional_expression(is_toplevel)
    elif self.token().is_keyword('def'):
      return self.parse_local_definition(is_toplevel, end)
    elif self.token().is_keyword('do'):
      return self.parse_do_expression(is_toplevel, end)
    else:
      value = self.parse_logical_expression()
      if (is_toplevel): self.expect_delimiter(';')
      return value

  def parse_logical_expression(self):
    exprs = [ self.parse_and_expression() ]
    while self.token().is_keyword('or'):
      self.expect_keyword('or')
      exprs.append(self.parse_and_expression())
    result = exprs[-1]
    for i in xrange(2, len(exprs) + 1):
      result = Conditional(exprs[-i], Thrue(), result)
    return result

  def parse_and_expression(self):
    exprs = [ self.parse_prefix_expression() ]
    while self.token().is_keyword('and'):
      self.expect_keyword('and')
      exprs.append(self.parse_prefix_expression())
    # We construct the resulting conditional expression backwards
    # To make the conditionals simple
    result = exprs[-1];
    for i in xrange(2, len(exprs) + 1):
      result = Conditional(exprs[-i], result, Fahlse())
    return result

  def parse_prefix_expression(self):
    if self.token().is_keyword('fn'):
      self.expect_keyword('fn')
      params = self.parse_params('(', ')')
      self.push_scope(params)
      expr = self.parse_function_body(False)
      self.pop_scope()
      return Lambda(params, expr)
    elif self.token().is_keyword('task'):
      self.expect_keyword('task')
      self.push_scope([])
      expr = self.parse_function_body(False)
      self.pop_scope();
      return Task(expr);
    else:
      return self.parse_operator_expression(None)

  def parse_operator_expression(self, end):
    expr = self.parse_call_expression()
    while self.token().is_operator() and (self.token().name != end):
      op = self.expect_operator()
      right = self.parse_call_expression()
      expr = Invoke(expr, op, Arguments([right], {}))
    return expr

  # <conditional-expression>
  #   -> 'if' '(' <expression> ')' <expression> ('else' <expression>)?
  def parse_conditional_expression(self, is_toplevel):
    self.expect_keyword('if')
    self.expect_delimiter('(')
    cond = self.parse_expression(False)
    self.expect_delimiter(')')
    then_part = self.parse_expression(is_toplevel)
    if self.token().is_keyword('else'):
      self.expect_keyword('else')
      else_part = self.parse_expression(is_toplevel)
    else:
      else_part = Void()
    return Conditional(cond, then_part, else_part)

  def at_call_prefix_start(self):
    return self.token().is_delimiter('.')   \
        or self.token().is_delimiter(u'·')  \
        or self.token().is_delimiter('(')

  # <call_expression>
  #   -> <atomic_expression> <arguments>
  #   -> <atomic_expression> '·' <atomic expression> <arguments>
  #   -> <atomic_expression> '.' $name <arguments>
  def parse_call_expression(self):
    expr = self.parse_unary_expression()
    while self.at_call_prefix_start():
      if self.token().is_delimiter('.'):
        self.expect_delimiter('.')
        name = self.expect_ident()
        args = self.parse_arguments('(', ')')
        expr = Invoke(expr, name, args)
      else:
        if self.token().is_delimiter(u'·'):
          self.expect_delimiter(u'·')
          recv = expr
          expr = self.parse_unary_expression()
          args = self.parse_arguments('(', ')')
          expr = Call(recv, expr, args)
        else:
          args = self.parse_arguments('(', ')')
          expr = Call(This(), expr, args)
    return expr

  def parse_unary_expression(self):
    if self.token().is_delimiter('~'):
      return self.parse_unquote_expression()
    else:
      return self.parse_atomic_expression()

  def parse_new_expression(self):
    self.expect_keyword('new')
    if self.token().is_delimiter('{'):
      constr = Global('Object')
      args = Arguments([], {})
    else:
      constr = self.parse_atomic_expression()
      args = self.parse_arguments('(', ')')
    if self.token().is_delimiter('{'):
      self.expect_delimiter('{')
      terms = [ ]
      while not self.token().is_delimiter('}'):
        name = self.expect_ident()
        self.expect_delimiter(':')
        value = self.parse_expression(False)
        terms.append((name, value))
        if not self.token().is_delimiter(','): break
        else: self.expect_delimiter(',')
      self.expect_delimiter('}')
      return Instantiate(constr, 'new', args, terms)
    else:
      return Invoke(constr, 'new', args)

  def resolve_identifier(self, name):
    return self.scope.lookup(name)

  def parse_string(self):
    terms = self.token().terms
    self.advance()
    if len(terms) == 0:
      return Literal(HEAP.new_string(""))
    elif len(terms) == 1 and (not isinstance(terms[0], Expression)):
      return Literal(HEAP.new_string(terms[0]))
    else:
      return Interpolated(terms)

  def parse_unquote_expression(self):
    self.expect_delimiter('~')
    current_quote_level = self.quote_level
    self.quote_level = current_quote_level.parent
    expr = self.parse_unary_expression()
    self.quote_level = current_quote_level
    index = current_quote_level.register_unquoted(expr)
    return Unquote(index)

  def parse_quote_expression(self):
    self.expect_operator('<')
    my_quote_level = QuoteLevel(self.quote_level)
    self.quote_level = my_quote_level
    value = self.parse_operator_expression('>')
    self.quote_level = self.quote_level.parent
    self.expect_operator('>')
    unquotes = my_quote_level.unquotes
    return Quote(value, unquotes)

  # <atomic_expression>
  #   -> $number
  #   -> $string
  #   -> 'this'
  #   -> 'true' | 'false' | 'null'
  #   -> '(' <expression> ')'
  def parse_atomic_expression(self):
    if self.token().is_number():
      value = self.token().value
      self.advance()
      return Literal(HEAP.new_number(value))
    elif self.token().is_string():
      return self.parse_string()
    elif self.token().is_ident():
      name = self.token().name
      self.advance()
      return self.resolve_identifier(name)
    elif self.token().is_keyword('this'):
      self.advance()
      return This()
    elif self.token().is_keyword('true'):
      self.advance()
      return Thrue()
    elif self.token().is_keyword('false'):
      self.advance()
      return Fahlse()
    elif self.token().is_keyword('null'):
      self.advance()
      return Null()
    elif self.token().is_keyword('new'):
      return self.parse_new_expression()
    elif self.token().is_delimiter('['):
      exprs = self.parse_expressions('[', ']')
      return Tuple(exprs)
    elif self.token().is_keyword('protocol'):
      return self.parse_protocol_declaration([])
    elif self.token().is_delimiter('('):
      self.expect_delimiter('(')
      value = self.parse_expression(False)
      self.expect_delimiter(')')
      return value
    elif self.token().is_operator('<'):
      return self.parse_quote_expression()
    elif self.token().is_operator():
      op = self.expect_circumfix_operator()
      match = circumfix_match(op)
      value = self.parse_operator_expression(match)
      self.expect_operator(match)
      return Invoke(value, op + match, Arguments([], {}))
    else:
      self.parse_error()

# -------------------------------
# --- S y n t a x   T r e e s ---
# -------------------------------

class SyntaxTree:
  pass

class Program(SyntaxTree):
  def __init__(self, decls):
    self.decls = decls
  def accept(self, visitor):
    visitor.visit_program(self)
  def traverse(self, visitor):
    for decl in self.decls:
      decl.accept(visitor)

class Definition(SyntaxTree):
  def __init__(self, name, value):
    self.name = name
    self.value = value
  def accept(self, visitor):
    visitor.visit_definition(self)
  def traverse(self, visitor):
    self.value.accept(visitor)

class BuiltinClass(SyntaxTree):
  def __init__(self, info, members, super):
    self.info = info
    self.members = members
    if super:
      self.super_name = super
    elif not self.info.name == u'Object':
      self.super_name = u'Object'
    self.signature = None
    self.image = None
  def get_signature(self):
    if not self.signature:
      tuple = HEAP.new_tuple(length = 1)
      self.signature_tuple = tuple
      self.signature = HEAP.new_signature(tuple)
      if self.image:
        tuple[0] = self.image
    return self.signature
  def set_image(self, value):
    assert not self.image
    self.image = value
    if self.signature:
      self.signature_tuple[0] = value
  def accept(self, visitor):
    visitor.visit_builtin_class(self)
  def traverse(self, visitor):
    for member in self.members:
      member.accept(visitor)

class SignatureProvider(SyntaxTree):
  def __init__(self):
    self.signature = None
  def get_signature(self):
    if not self.signature:
      tuple = HEAP.new_tuple(length = 1)
      self.signature_tuple = tuple
      self.signature = HEAP.new_signature(tuple)
      if self.get_image():
        tuple[0] = self.get_image()
    return self.signature

class Protocol(SignatureProvider):
  def __init__(self, name, members, super):
    SignatureProvider.__init__(self)
    self.name = name
    self.members = members
    if super:
      self.super_name = super
    elif not self.name == 'Object':
      self.super_name = 'Object'
    else:
      self.super_name = None
    self.image = None
  def get_image(self):
    return self.image
  def set_image(self, value):
    assert not self.image
    self.image = value
    if self.signature:
      self.signature_tuple[0] = value    
  def accept(self, visitor):
    visitor.visit_protocol(self)
  def traverse(self, visitor):
    for member in self.members:
      member.accept(visitor)
  def quote(self):
    methods = HEAP.new_tuple(values = [ m.quote() for m in self.members ])
    super = NAMESPACE[self.super_name]
    return HEAP.new_protocol_expression(HEAP.new_string(self.name), methods, super.image)

class Selector(SyntaxTree):
  def __init__(self, name, argc, keywords):
    self.name = name
    self.argc = argc
    self.keywords = keywords

class Method(SyntaxTree):
  def __init__(self, is_static, name, fun):
    self.is_static = is_static
    self.name = name
    self.fun = fun
  def accept(self, visitor):
    visitor.visit_method(self)
  def traverse(self, visitor):
    self.fun.accept(visitor)
  def compile(self, protocol = None):
    body = self.fun.compile()
    if protocol:
      signature = protocol.get_signature()
    else:
      signature = HEAP.empty_signature
    name = HEAP.new_string(self.name)
    argc = len(self.fun.params)
    return HEAP.new_method(HEAP.new_selector(name, argc), signature, body)
  def quote(self):
    fun = self.fun.quote()
    if self.is_static: is_static = HEAP.true
    else: is_static = HEAP.false
    name = HEAP.new_string(self.name)
    argc = len(self.fun.params)
    return HEAP.new_method_expression(HEAP.new_selector(name, argc), fun, is_static)

class Expression(SyntaxTree):
  def is_identifier(self):
    return False

def quote_value(val):
  if type(val) is int: return val
  else: return val

class Literal(Expression):
  def __init__(self, value):
    self.value = value
  def accept(self, visitor):
    visitor.visit_literal(self)
  def traverse(self, visitor):
    pass
  def __repr__(self):
    return str(self.value)
  def quote(self):
    val = quote_value(self.value)
    return HEAP.new_literal_expression(val)

class Interpolated(Expression):
  def __init__(self, terms):
    assert len(terms) > 1
    self.terms = terms
  def accept(self, visitor):
    visitor.visit_interpolated(self)
  def traverse(self, visitor):
    for term in self.terms:
      if isinstance(term, Expression):
        term.accept(visitor)
  def quote(self):
    exprs = []
    for term in self.terms:
      if isinstance(term, Expression):
        exprs.append(term.quote())
      else:
        exprs.append(HEAP.new_string(term))
    return HEAP.new_interpolate_expression(HEAP.new_tuple(values = exprs))

class Tuple(Expression):
  def __init__(self, values):
    assert type(values) is list
    self.values = values
  def accept(self, visitor):
    visitor.visit_tuple(self)
  def traverse(self, visitor):
    for value in self.values:
      value.accept(visitor)
  def quote(self):
    values = HEAP.new_tuple(values = [ value.quote() for value in self.values ])
    return HEAP.new_tuple_expression(values)

def compile_lambda(params, body):
  syms = [ HEAP.get_symbol(param) for param in params ]
  expr = HEAP.new_lambda_expression(HEAP.new_tuple(values = syms), body.quote())
  return HEAP.new_lambda(len(params), expr)

def compile_params(params):
  result = []
  for param in params:
    if isinstance(param, Symbol):
      result.append(HEAP.get_symbol(param))
    else:
      result.append(param.quote())
  return HEAP.new_tuple(values = result)

class Lambda(Expression):
  def __init__(self, params, body):
    self.params = params
    self.body = body
  def accept(self, visitor):
    visitor.visit_lambda(self)
  def traverse(self, visitor):
    self.body.accept(visitor)
  def quote(self):
    body = self.body.quote()
    params = compile_params(self.params)
    return HEAP.new_lambda_expression(params, body)
  def __repr__(self):
    return str(self.params) + ' -> ' + str(self.body)
  def compile(self):
    return compile_lambda(self.params, self.body)

class BuiltinCall(Expression):
  def __init__(self, argc, index):
    self.argc = argc
    self.index = index
  def accept(self, visitor):
    visitor.visit_builtin_lambda(self)
  def traverse(self, visitor):
    pass
  def quote(self):
    return HEAP.new_builtin_call(self.argc, self.index)

class ExternalCall(Expression):
  def __init__(self, argc, name):
    self.argc = argc
    self.name = name
  def accept(self, visitor):
    visitor.visit_external_call(self)
  def traverse(self, visitor):
    pass
  def quote(self):
    return HEAP.new_external_call(self.argc, HEAP.new_string(self.name))

class Identifier(Expression):
  def accept(self, visitor):
    visitor.visit_identifier(self)
  def is_identifier(self):
    return True
  def traverse(self, visitor):
    pass

class Global(Identifier):
  def __init__(self, name):
    self.name = name
  def __repr__(self):
    return "$" + self.name
  def quote(self):
    return HEAP.new_global(HEAP.new_string(self.name))

class Local(Identifier):
  def __init__(self, symbol):
    self.symbol = symbol
    self.name = symbol.name
  def __repr__(self):
    return "%" + str(self.symbol)
  def quote(self):
    return self.symbol.quote()

class Call(Expression):
  def __init__(self, recv, fun, args):
    self.recv = recv
    self.fun = fun
    self.args = args
  def accept(self, visitor):
    visitor.visit_call(self)
  def traverse(self, visitor):
    self.recv.accept(visitor)
    self.fun.accept(visitor)
    self.args.accept(visitor)
  def __repr__(self):
    return str(self.recv) + "·" + str(self.fun) + exprs_repr(self.args)
  def quote(self):
    recv = self.recv.quote()
    fun = self.fun.quote()
    args = self.args.quote()
    return HEAP.new_call_expression(recv, fun, args)

class Raise(Expression):
  def __init__(self, name, args):
    self.name = name
    self.args = args
  def accept(self, visitor):
    visitor.visit_raise(self)
  def traverse(self, visitor):
    self.args.accept(visitor)
  def quote(self):
    name = HEAP.new_string(self.name)
    args = self.args.quote()
    return HEAP.new_raise_expression(name, args)

class DoOnExpression(Expression):
  def __init__(self, value, clauses):
    self.value = value
    self.clauses = clauses
  def accept(self, visitor):
    visitor.visit_do_on_expression(self)
  def traverse(self, visitor):
    self.value.accept(visitor)
    for clause in self.clauses: clause.accept(visitor)
  def quote(self):
    value = self.value.quote()
    clauses = HEAP.new_tuple(values = [ clause.quote() for clause in self.clauses ])
    return HEAP.new_do_on_expression(value, clauses)

class OnClause(Expression):
  def __init__(self, name, fun):
    self.name = name
    self.fun = fun
  def accept(self, visitor):
    visitor.visit_on_clause(self)
  def traverse(self, visitor):
    self.fun.accept(visitor)
  def quote(self):
    name = HEAP.new_string(self.name)
    fun = self.fun.quote()
    return HEAP.new_on_clause(name, fun)

class Quote(Expression):
  def __init__(self, value, unquotes):
    self.value = value
    self.unquotes = unquotes
  def accept(self, visitor):
    visitor.visit_quote(self)
  def traverse(self, visitor):
    self.value.accept(visitor)
    for unquote in self.unquotes:
      unquote.accept(visitor)
  def __repr__(self):
    return "'" + str(self.unquotes) + ":(" + str(self.value) + ')'
  def quote(self):
    unquotes = HEAP.new_tuple(values = [ unq.quote() for unq in self.unquotes ])
    return HEAP.new_quote_expression(self.value.quote(), unquotes)

class Unquote(Expression):
  def __init__(self, value):
    self.value = value
    self.name = None
  def accept(self, visitor):
    visitor.visit_unquote(self)
  def traverse(self, visitor):
    pass
  def __repr__(self):
    return "," + str(self.value)
  def quote(self):
    return HEAP.new_unquote_expression(self.value)

def exprs_repr(args):
  return '(' + ', '.join([str(arg) for arg in args.args]) + ')'

class Arguments(SyntaxTree):
  def __init__(self, args, keywords):
    self.args = args
    self.keywords = keywords
  def accept(self, visitor):
    visitor.visit_arguments(self)
  def length(self):
    return len(self.args)
  def traverse(self, visitor):
    for arg in self.args:
      arg.accept(visitor)
  def quote(self):
    args = HEAP.new_tuple(values = [ arg.quote() for arg in self.args ])
    if len(self.keywords) == 0:
      keywords = HEAP.empty_tuple
    else:
      sorted_keywords = self.keywords.keys()
      sorted_keywords.sort();
      keywords_list = [ ]
      for word in sorted_keywords:
        index = self.keywords[word]
        word_obj = HEAP.new_string(word)
        index_obj = tag_as_smi(index)
        keywords_list.append(HEAP.new_tuple(values = [word_obj, index_obj]))
      keywords = HEAP.new_tuple(values = keywords_list)
    return HEAP.new_arguments(args, keywords)

class Invoke(Expression):
  def __init__(self, recv, name, args):
    self.recv = recv
    self.name = name
    self.args = args
  def accept(self, visitor):
    visitor.visit_invoke(self)
  def traverse(self, visitor):
    self.recv.accept(visitor)
    self.args.accept(visitor)
  def __repr__(self):
    return str(self.recv) + '.' + self.name + exprs_repr(self.args)
  def quote(self):
    recv = self.recv.quote()
    name = HEAP.new_string(self.name)
    selector = HEAP.new_selector(name, self.args.length())
    args = self.args.quote()
    return HEAP.new_invoke_expression(recv, selector, args)

class Instantiate(Expression):
  def __init__(self, recv, name, args, terms):
    self.recv = recv
    self.name = name
    self.args = args
    self.terms = terms
  def accept(self, visitor):
    visitor.visit_instantiate(self)
  def traverse(self, visitor):
    self.recv.accept(visitor)
    self.args.accept(visitor)
    for (name, value) in self.terms:
      value.accept(visitor)
  def quote(self):
    recv = self.recv.quote()
    name = HEAP.new_string(self.name)
    args = self.args.quote()
    term_list = [ ]
    for (key, value) in self.terms:
      term_list.append(HEAP.new_string(key))
      term_list.append(value.quote())
    terms = HEAP.new_tuple(values = term_list)
    return HEAP.new_instantiate_expression(recv, name, args, terms)

class This(Expression):
  def accept(self, visitor):
    visitor.visit_this(self)
  def traverse(self, visitor):
    pass
  def __repr__(self):
    return "this"
  def quote(self):
    return HEAP.new_this_expression()

class Void(Expression):
  def accept(self, visitor):
    visitor.visit_void(self)
  def traverse(self, visitor):
    pass
  def quote(self):
    return HEAP.new_literal_expression(HEAP.get_root(ROOT_INDEX['VoidValue']))

class Null(Expression):
  def accept(self, visitor):
    visitor.visit_null(self)
  def traverse(self, visitor):
    pass
  def quote(self):
    return HEAP.new_literal_expression(HEAP.get_root(ROOT_INDEX['NullValue']))

class Thrue(Expression):
  def accept(self, visitor):
    visitor.visit_true(self)
  def __repr__(self):
    return "true"
  def traverse(self, visitor):
    pass
  def quote(self):
    return HEAP.new_literal_expression(HEAP.get_root(ROOT_INDEX['TrueValue']))

class Fahlse(Expression):
  def accept(self, visitor):
    visitor.visit_false(self)
  def __repr__(self):
    return "false"
  def traverse(self, visitor):
    pass
  def quote(self):
    return HEAP.new_literal_expression(HEAP.get_root(ROOT_INDEX['FalseValue']))

class Return(Expression):
  def __init__(self, value):
    self.value = value
  def accept(self, visitor):
    visitor.visit_return(self)
  def traverse(self, visitor):
    self.value.accept(visitor)
  def __repr__(self):
    return "^" + str(self.value)
  def quote(self):
    return HEAP.new_return_expression(self.value.quote())

class Yield(Expression):
  def __init__(self, value):
    self.value = value
  def accept(self, visitor):
    visitor.visit_yield(self)
  def traverse(self, visitor):
    self.value.accept(visitor)
  def quote(self):
    return HEAP.new_yield_expression(self.value.quote())

class Task(Expression):
  def __init__(self, body):
    self.body = body
  def accept(self, visitor):
    visitor.visit_task(self)
  def traverse(self, visitor):
    self.body.accept(visitor)
  def quote(self):
    return HEAP.new_task_expression(self.body.quote())

class Sequence(Expression):
  def __init__(self, exprs):
    self.exprs = exprs
  def accept(self, visitor):
    visitor.visit_sequence(self)
  def traverse(self, visitor):
    for expr in self.exprs:
      expr.accept(visitor)
  def quote(self):
    exprs = HEAP.new_tuple(values = [ expr.quote() for expr in self.exprs ])
    return HEAP.new_sequence_expression(exprs)

class LocalDefinition(Expression):
  def __init__(self, name, value, body):
    self.name = name
    self.value = value
    self.body = body
  def accept(self, visitor):
    visitor.visit_local_definition(self)
  def traverse(self, visitor):
    self.value.accept(visitor)
    self.body.accept(visitor)
  def quote(self):
    name = HEAP.get_symbol(self.name)
    value = self.value.quote()
    body = self.body.quote()
    return HEAP.new_local_definition(name, value, body)

class Conditional(Expression):
  def __init__(self, cond, then_part, else_part):
    self.cond = cond
    self.then_part = then_part
    self.else_part = else_part
  def __repr__(self):
    return 'if ' + str(self.cond) + ' then ' + str(self.then_part) + ' else ' + str(self.else_part)
  def accept(self, visitor):
    visitor.visit_conditional(self)
  def traverse(self, visitor):
    self.cond.accept(visitor)
    self.then_part.accept(visitor)
    self.else_part.accept(visitor)
  def quote(self):
    return HEAP.new_conditional(self.cond.quote(), self.then_part.quote(), self.else_part.quote())

class Symbol(object):
  def __init__(self, name):
    self.name = name
  def __repr__(self):
    return "[" + self.name + " " + str(self.__hash__()) + "]"
  def quote(self):
    return HEAP.get_symbol(self)

# ---------------------------
# --- H e a p   I m a g e ---
# ---------------------------

def tag_as_smi(value):
  return value << 2

def has_smi_tag(value):
  return value & 0x3 is 0

def tag_as_object(value):
  assert value & 0x3 is 0
  return value | 0x1

def has_object_tag(value):
  return value & 0x3 is 0x1

def untag_object(value):
  assert (value & 0x3) is 0x1
  return value & ~0x3;

POINTER_SIZE = 4

class Heap:
  kRootCount  = 51
  def __init__(self):
    self.capacity = 1024
    self.cursor = 0
    self.memory = self.capacity * [ tag_as_smi(0) ]
    self.dicts = [ ]
    self.root_object_cache = { }
    self.symbol_object_cache = { }
  def initialize(self):
    self.roots = self.new_tuple(Heap.kRootCount)
    self.toplevel = self.new_dictionary()
    self.set_root(ROOT_INDEX['Toplevel'], self.toplevel)
    self.context = self.new_context()
    self.empty_tuple = self.new_tuple(length = 0)
    self.empty_signature = self.new_signature(self.empty_tuple)
    self.void = self.get_root(ROOT_INDEX['VoidValue'])
    self.true = self.get_root(ROOT_INDEX['TrueValue'])
    self.false = self.get_root(ROOT_INDEX['FalseValue'])
  def set_raw(self, offset, value):
    self.memory[offset] = value
  def set_root(self, index, value):
    assert 0 <= index < Heap.kRootCount;
    self.roots[index] = value
  def __setitem__(self, offset, value):
    if type(value) is int:
      self.memory[offset] = tag_as_smi(value)
    else:
      addr = value.addr
      self.memory[offset] = tag_as_object(addr)

  # --- A l l o c a t i o n ---

  def allocate(self, type, size):
    if self.cursor + size > self.capacity:
      new_capacity = self.capacity * 2
      new_memory = new_capacity * [ tag_as_smi(0) ]
      for i in xrange(0, self.cursor):
        new_memory[i] = self.memory[i]
      self.capacity = new_capacity
      self.memory = new_memory
    addr = self.cursor * POINTER_SIZE
    self.cursor += size
    ImageObject(addr).set_layout(type)
    return addr

  def new_context(self):
    return ImageContext(self.allocate(CONTEXT_TYPE, ImageContext_Size))

  def new_protocol(self, name):
    return ImageProtocol(self.allocate(PROTOCOL_TYPE, ImageProtocol_Size), name)

  def new_layout(self, proto, methods, instance_type, field_count):
    return ImageLayout(self.allocate(LAYOUT_TYPE, ImageLayout_Size), proto, methods, instance_type, field_count)

  def get_root(self, index):
    if index in self.root_object_cache:
      return self.root_object_cache[index]
    result = ImageRoot(self.allocate(SINGLETON_TYPE, ImageRoot_Size), index)
    self.root_object_cache[index] = result
    return result

  def get_symbol(self, symbol):
    if symbol in self.symbol_object_cache:
      return self.symbol_object_cache[symbol]
    name = HEAP.new_string(symbol.name)
    result = ImageSymbol(self.allocate(SYMBOL_TYPE, ImageSymbol_Size), name)
    self.symbol_object_cache[symbol] = result
    return result

  def new_lambda(self, argc, expr):
    return ImageLambda(self.allocate(LAMBDA_TYPE, ImageLambda_Size), argc, expr, self.context)

  def new_method(self, name, signature, body):
    return ImageMethod(self.allocate(METHOD_TYPE, ImageMethod_Size), name, signature, body)

  def new_signature(self, elements):
    return ImageSignature(self.allocate(SIGNATURE_TYPE, ImageSignature_Size), elements)

  def new_selector(self, name, argc):
    return ImageSelector(self.allocate(SELECTOR_TYPE, ImageSelector_Size), name, argc)

  def new_lambda_expression(self, params, body):
    return ImageLambdaExpression(self.allocate(LAMBDA_EXPRESSION_TYPE, ImageLambdaExpression_Size), params, body)

  def new_interpolate_expression(self, terms):
    return ImageInterpolateExpression(self.allocate(INTERPOLATE_EXPRESSION_TYPE, ImageInterpolateExpression_Size), terms)

  def new_local_definition(self, symbol, value, body):
    return ImageLocalDefinition(self.allocate(LOCAL_DEFINITION_TYPE, ImageLocalDefinition_Size), symbol, value, body)

  def new_builtin_call(self, argc, index):
    return ImageBuiltinCall(self.allocate(BUILTIN_CALL_TYPE, ImageBuiltinCall_Size), argc, index)

  def new_external_call(self, argc, name):
    return ImageExternalCall(self.allocate(EXTERNAL_CALL_TYPE, ImageExternalCall_Size), argc, name)

  def new_quote_expression(self, value, unquotes):
    return ImageQuoteExpression(self.allocate(QUOTE_EXPRESSION_TYPE, ImageQuoteExpression_Size), value, unquotes)

  def new_unquote_expression(self, value):
    return ImageUnquoteExpression(self.allocate(UNQUOTE_EXPRESSION_TYPE, ImageUnquoteExpression_Size), value)

  def new_this_expression(self):
    return ImageThisExpression(self.allocate(THIS_EXPRESSION_TYPE, ImageThisExpression_Size))

  def new_number(self, value):
    return value

  def new_dictionary(self):
    result = ImageDictionary(self.allocate(DICTIONARY_TYPE, ImageDictionary_Size))
    self.dicts.append(result)
    return result

  def new_tuple(self, length = None, values = None):
    if length is None: length = len(values)
    result = ImageTuple(self.allocate(TUPLE_TYPE, ImageTuple_HeaderSize + length), length)
    if not values is None:
      for i in xrange(length):
        result[i] = values[i]
    return result

  def new_string(self, contents):
    length = len(contents)
    result = ImageString(self.allocate(STRING_TYPE, ImageString_HeaderSize + length), length)
    for i in xrange(length):
      result[i] = ord(contents[i])
    return result

  def new_code(self, contents):
    length = len(contents)
    result = ImageCode(self.allocate(CODE_TYPE, ImageCode_HeaderSize + length), length)
    for i in xrange(length):
      result[i] = contents[i]
    return result

  def new_literal_expression(self, value):
    return ImageLiteralExpression(self.allocate(LITERAL_EXPRESSION_TYPE, ImageLiteralExpression_Size), value)

  def new_arguments(self, args, keywords):
    return ImageArguments(self.allocate(ARGUMENTS_TYPE, ImageArguments_Size), args, keywords)

  def new_invoke_expression(self, recv, name, args):
    return ImageInvokeExpression(self.allocate(INVOKE_EXPRESSION_TYPE, ImageInvokeExpression_Size), recv, name, args)

  def new_call_expression(self, recv, fun, args):
    return ImageCallExpression(self.allocate(CALL_EXPRESSION_TYPE, ImageCallExpression_Size), recv, fun, args)

  def new_raise_expression(self, name, args):
    return ImageRaiseExpression(self.allocate(RAISE_EXPRESSION_TYPE, ImageRaiseExpression_Size), name, args)

  def new_on_clause(self, name, fun):
    return ImageOnClause(self.allocate(ON_CLAUSE_TYPE, ImageOnClause_Size), name, fun)

  def new_do_on_expression(self, value, clauses):
    return ImageDoOnExpression(self.allocate(DO_ON_EXPRESSION_TYPE, ImageDoOnExpression_Size), value, clauses)

  def new_conditional(self, cond, then_part, else_part):
    return ImageConditionalExpression(self.allocate(CONDITIONAL_EXPRESSION_TYPE, ImageConditionalExpression_Size), cond, then_part, else_part)

  def new_protocol_expression(self, name, methods, super):
    return ImageProtocolExpression(self.allocate(PROTOCOL_EXPRESSION_TYPE, ImageProtocolExpression_Size), name, methods, super)

  def new_return_expression(self, value):
    return ImageReturnExpression(self.allocate(RETURN_EXPRESSION_TYPE, ImageReturnExpression_Size), value)

  def new_sequence_expression(self, exprs):
    return ImageSequenceExpression(self.allocate(SEQUENCE_EXPRESSION_TYPE, ImageSequenceExpression_Size), exprs)

  def new_method_expression(self, name, fun, is_static):
    return ImageMethodExpression(self.allocate(METHOD_EXPRESSION_TYPE, ImageMethodExpression_Size), name, fun, is_static)

  def new_tuple_expression(self, exprs):
    return ImageTupleExpression(self.allocate(TUPLE_EXPRESSION_TYPE, ImageTupleExpression_Size), exprs)

  def new_instantiate_expression(self, recv, name, args, terms):
    return ImageInstantiateExpression(self.allocate(INSTANTIATE_EXPRESSION_TYPE, ImageInstantiateExpression_Size), recv, name, args, terms)

  def new_global(self, name):
    return ImageGlobalExpression(self.allocate(GLOBAL_EXPRESSION_TYPE, ImageGlobalExpression_Size), name)

  # --- A c c e s s ---

  def set_raw_field(self, obj, offset, value):
    assert type(value) is int
    self.set_raw((obj.addr / POINTER_SIZE) + offset, value)

  def set_field(self, obj, offset, value):
    self[(obj.addr / POINTER_SIZE) + offset] = value

  def get_address(self, obj, offset, Type):
    value = self[(obj.addr / POINTER_SIZE) + offset]
    if has_smi_tag(value): return value >> 2
    else: return Type(untag_object(value))

  def flush(self):
    for dict in self.dicts:
      dict.commit()
    return self.memory[:self.cursor]

  def write_to(self, file):
    buffer = self.flush()
    def write(value):
      output.write(struct.pack('<I', value))
    output = open(file, 'wb')
    write(4206546606L)
    write(len(buffer))
    write(Heap.kRootCount)
    write(tag_as_object(self.roots.addr))
    for value in buffer:
      write(value)
    output.close()

class ImageValue:
  pass

class ImageObject(ImageValue):
  def __init__(self, addr):
    self.addr = addr
  def set_layout(self, value):
    HEAP.set_field(self, ImageObject_LayoutOffset, value)

class ImageProtocol(ImageObject):
  def __init__(self, addr, name=None):
    ImageObject.__init__(self, addr)
    if name:
      self.set_name(name)
  def set_methods(self, value):
    HEAP.set_field(self, ImageProtocol_MethodsOffset, value)
  def set_name(self, value):
    HEAP.set_field(self, ImageProtocol_NameOffset, value)
  def set_super(self, value):
    HEAP.set_field(self, ImageProtocol_SuperOffset, value)

class ImageLayout(ImageObject):
  def __init__(self, addr, proto, methods, instance_type, field_count):
    ImageObject.__init__(self, addr)
    HEAP.set_field(self, ImageLayout_ProtocolOffset, proto)
    HEAP.set_field(self, ImageLayout_MethodsOffset, methods)
    HEAP.set_raw_field(self, ImageLayout_InstanceTypeOffset, instance_type)
    HEAP.set_raw_field(self, ImageLayout_FieldCountOffset, field_count)

class ImageTuple(ImageObject):
  def __init__(self, addr, length):
    ImageObject.__init__(self, addr)
    self.set_length(length)
  def set_length(self, value):
    HEAP.set_raw_field(self, ImageTuple_LengthOffset, value)
  def __setitem__(self, index, value):
    HEAP.set_field(self, ImageTuple_HeaderSize + index, value)

class ImageLambda(ImageObject):
  def __init__(self, addr, argc, tree, context):
    ImageObject.__init__(self, addr)
    self.set_argc(argc)
    self.set_code(0)
    self.set_literals(0)
    self.set_context(context)
    self.set_tree(tree)
  def set_argc(self, value):
    HEAP.set_raw_field(self, ImageLambda_ArgcOffset, value)
  def set_code(self, value):
    HEAP.set_field(self, ImageLambda_CodeOffset, value)
  def set_literals(self, value):
    HEAP.set_field(self, ImageLambda_LiteralsOffset, value)
  def set_tree(self, value):
    HEAP.set_field(self, ImageLambda_TreeOffset, value)
  def set_context(self, value):
    HEAP.set_field(self, ImageLambda_ContextOffset, value)

class ImageContext(ImageObject):
  def __init__(self, addr):
    ImageObject.__init__(self, addr)

class ImageBuiltinCall(ImageObject):
  def __init__(self, addr, argc, index):
    ImageObject.__init__(self, addr)
    HEAP.set_raw_field(self, ImageBuiltinCall_ArgcOffset, argc)
    HEAP.set_raw_field(self, ImageBuiltinCall_IndexOffset, index)

class ImageExternalCall(ImageObject):
  def __init__(self, addr, argc, name):
    ImageObject.__init__(self, addr)
    HEAP.set_field(self, ImageBuiltinCall_ArgcOffset, argc)
    HEAP.set_field(self, ImageBuiltinCall_IndexOffset, name)

class ImageMethod(ImageObject):
  def __init__(self, addr, selector, signature, body):
    ImageObject.__init__(self, addr)
    HEAP.set_field(self, ImageMethod_SelectorOffset, selector)
    HEAP.set_field(self, ImageMethod_SignatureOffset, signature)
    HEAP.set_field(self, ImageMethod_LambdaOffset, body)

class ImageSignature(ImageObject):
  def __init__(self, addr, parameters):
    ImageObject.__init__(self, addr)
    HEAP.set_field(self, ImageSignature_ParametersOffset, parameters)

class ImageSelector(ImageObject):
  def __init__(self, addr, name, argc):
    ImageObject.__init__(self, addr)
    HEAP.set_field(self, ImageSelector_NameOffset, name)
    HEAP.set_field(self, ImageSelector_ArgcOffset, argc)

class ImageString(ImageObject):
  def __init__(self, addr, length):
    ImageObject.__init__(self, addr)
    self.set_length(length)
  def set_length(self, value):
    HEAP.set_raw_field(self, ImageString_LengthOffset, value)
  def __setitem__(self, index, value):
    HEAP.set_raw_field(self, ImageString_HeaderSize + index, value)

class ImageSymbol(ImageObject):
  def __init__(self, addr, name):
    ImageObject.__init__(self, addr)
    self.set_name(name)
  def set_name(self, value):
    HEAP.set_field(self, ImageSymbol_NameOffset, value)

class ImageCode(ImageObject):
  def __init__(self, addr, length):
    ImageObject.__init__(self, addr)
    self.set_length(length)
  def set_length(self, value):
    HEAP.set_raw_field(self, ImageCode_LengthOffset, value)
  def __setitem__(self, index, value):
    HEAP.set_raw_field(self, ImageCode_HeaderSize + index, value)

class ImageDictionary(ImageObject):
  def __init__(self, addr):
    ImageObject.__init__(self, addr)
    self.entries = { }
  def __setitem__(self, name, value):
    self.entries[name] = value
  def set_table(self, value):
    HEAP.set_field(self, ImageDictionary_TableOffset, value)
  def commit(self):
    table = HEAP.new_tuple(len(self.entries) * 2)
    self.entries.keys().sort()
    cursor = 0
    for (key, value) in self.entries.items():
      table[cursor] = key
      cursor += 1
      table[cursor] = value
      cursor += 1
    self.set_table(table)

class ImageRoot(ImageObject):
  def __init__(self, addr, index):
    ImageObject.__init__(self, addr)
    self.set_index(index)
  def set_index(self, value):
    HEAP.set_raw_field(self, ImageRoot_IndexOffset, value)

class ImageSyntaxTree(ImageObject):
  def __init__(self, addr):
    ImageObject.__init__(self, addr)

class ImageLiteralExpression(ImageSyntaxTree):
  def __init__(self, addr, value):
    ImageSyntaxTree.__init__(self, addr)
    self.set_value(value)
  def set_value(self, value):
    HEAP.set_field(self, ImageLiteralExpression_ValueOffset, value)

class ImageInvokeExpression(ImageSyntaxTree):
  def __init__(self, addr, recv, selector, args):
    ImageSyntaxTree.__init__(self, addr)
    HEAP.set_field(self, ImageInvokeExpression_ReceiverOffset, recv)
    HEAP.set_field(self, ImageInvokeExpression_SelectorOffset, selector)
    HEAP.set_field(self, ImageInvokeExpression_ArgumentsOffset, args)

class ImageArguments(ImageSyntaxTree):
  def __init__(self, addr, args, keywords):
    ImageSyntaxTree.__init__(self, addr)
    HEAP.set_field(self, ImageArguments_ArgumentsOffset, args)
    HEAP.set_field(self, ImageArguments_KeywordsOffset, keywords)

class ImageInstantiateExpression(ImageSyntaxTree):
  def __init__(self, addr, recv, name, args, terms):
    ImageSyntaxTree.__init__(self, addr)
    self.set_recv(recv)
    self.set_name(name)
    self.set_args(args)
    self.set_terms(terms)
  def set_recv(self, value):
    HEAP.set_field(self, ImageInstantiateExpression_ReceiverOffset, value)
  def set_name(self, value):
    HEAP.set_field(self, ImageInstantiateExpression_NameOffset, value)
  def set_args(self, value):
    HEAP.set_field(self, ImageInstantiateExpression_ArgumentsOffset, value)
  def set_terms(self, value):
    HEAP.set_field(self, ImageInstantiateExpression_TermsOffset, value)

class ImageCallExpression(ImageSyntaxTree):
  def __init__(self, addr, recv, fun, args):
    ImageSyntaxTree.__init__(self, addr)
    self.set_recv(recv)
    self.set_fun(fun)
    self.set_args(args)
  def set_recv(self, value):
    HEAP.set_field(self, ImageCallExpression_ReceiverOffset, value)
  def set_fun(self, value):
    HEAP.set_field(self, ImageCallExpression_FunctionOffset, value)
  def set_args(self, value):
    HEAP.set_field(self, ImageCallExpression_ArgumentsOffset, value)

class ImageRaiseExpression(ImageSyntaxTree):
  def __init__(self, addr, name, args):
    ImageSyntaxTree.__init__(self, addr)
    self.set_name(name)
    self.set_args(args)
  def set_name(self, value):
    HEAP.set_field(self, ImageRaiseExpression_NameOffset, value)
  def set_args(self, value):
    HEAP.set_field(self, ImageRaiseExpression_ArgumentsOffset, value)

class ImageOnClause(ImageSyntaxTree):
  def __init__(self, addr, name, fun):
    ImageSyntaxTree.__init__(self, addr)
    self.set_name(name)
    self.set_fun(fun)
  def set_name(self, value):
    HEAP.set_field(self, ImageOnClause_NameOffset, value)
  def set_fun(self, value):
    HEAP.set_field(self, ImageOnClause_LambdaOffset, value)

class ImageDoOnExpression(ImageSyntaxTree):
  def __init__(self, addr, value, clauses):
    ImageSyntaxTree.__init__(self, addr)
    self.set_value(value)
    self.set_clauses(clauses)
  def set_value(self, value):
    HEAP.set_field(self, ImageDoOnExpression_ValueOffset, value)
  def set_clauses(self, value):
    HEAP.set_field(self, ImageDoOnExpression_ClausesOffset, value)

class ImageConditionalExpression(ImageSyntaxTree):
  def __init__(self, addr, cond, then_part, else_part):
    ImageSyntaxTree.__init__(self, addr)
    self.set_cond(cond)
    self.set_then_part(then_part)
    self.set_else_part(else_part)
  def set_cond(self, value):
    HEAP.set_field(self, ImageConditionalExpression_ConditionOffset, value)
  def set_then_part(self, value):
    HEAP.set_field(self, ImageConditionalExpression_ThenPartOffset, value)
  def set_else_part(self, value):
    HEAP.set_field(self, ImageConditionalExpression_ElsePartOffset, value)


class ImageProtocolExpression(ImageSyntaxTree):
  def __init__(self, addr, name, methods, super):
    ImageSyntaxTree.__init__(self, addr)
    HEAP.set_field(self, ImageProtocolExpression_NameOffset, name)
    HEAP.set_field(self, ImageProtocolExpression_MethodsOffset, methods)
    HEAP.set_field(self, ImageProtocolExpression_SuperOffset, super)

class ImageReturnExpression(ImageSyntaxTree):
  def __init__(self, addr, value):
    ImageSyntaxTree.__init__(self, addr)
    HEAP.set_field(self, ImageReturnExpression_ValueOffset, value)

class ImageSequenceExpression(ImageSyntaxTree):
  def __init__(self, addr, exprs):
    ImageSyntaxTree.__init__(self, addr)
    HEAP.set_field(self, ImageSequenceExpression_ExpressionsOffset, exprs)

class ImageLocalDefinition(ImageSyntaxTree):
  def __init__(self, addr, symbol, value, body):
    ImageSyntaxTree.__init__(self, addr)
    self.set_symbol(symbol)
    self.set_value(value)
    self.set_body(body)
  def set_symbol(self, value):
    HEAP.set_field(self, ImageLocalDefinition_SymbolOffset, value)
  def set_value(self, value):
    HEAP.set_field(self, ImageLocalDefinition_ValueOffset, value)
  def set_body(self, value):
    HEAP.set_field(self, ImageLocalDefinition_BodyOffset, value)

class ImageTupleExpression(ImageSyntaxTree):
  def __init__(self, addr, exprs):
    ImageSyntaxTree.__init__(self, addr)
    self.set_values(exprs)
  def set_values(self, value):
    HEAP.set_field(self, ImageTupleExpression_ValuesOffset, value)

class ImageInterpolateExpression(ImageSyntaxTree):
  def __init__(self, addr, terms):
    ImageSyntaxTree.__init__(self, addr)
    self.set_terms(terms)
  def set_terms(self, value):
    HEAP.set_field(self, ImageInterpolateExpression_TermsOffset, value)

class ImageMethodExpression(ImageSyntaxTree):
  def __init__(self, addr, selector, fun, is_static):
    ImageSyntaxTree.__init__(self, addr)
    HEAP.set_field(self, ImageMethodExpression_SelectorOffset, selector)
    HEAP.set_field(self, ImageMethodExpression_LambdaOffset, fun)
    HEAP.set_field(self, ImageMethodExpression_IsStaticOffset, is_static)

class ImageLambdaExpression(ImageSyntaxTree):
  def __init__(self, addr, params, body):
    ImageSyntaxTree.__init__(self, addr)
    self.set_params(params)
    self.set_body(body)
  def set_params(self, value):
    HEAP.set_field(self, ImageLambdaExpression_ParamsOffset, value)
  def set_body(self, value):
    HEAP.set_field(self, ImageLambdaExpression_BodyOffset, value)

class ImageGlobalExpression(ImageSyntaxTree):
  def __init__(self, addr, name):
    ImageSyntaxTree.__init__(self, addr)
    self.set_name(name)
  def set_name(self, value):
    HEAP.set_field(self, ImageGlobalExpression_NameOffset, value)

class ImageThisExpression(ImageSyntaxTree):
  def __init__(self, addr):
    ImageSyntaxTree.__init__(self, addr)

class ImageQuoteExpression(ImageSyntaxTree):
  def __init__(self, addr, value, unquotes):
    ImageSyntaxTree.__init__(self, addr)
    self.set_value(value)
    self.set_unquotes(unquotes)
  def set_value(self, value):
    HEAP.set_field(self, ImageQuoteExpression_ValueOffset, value)
  def set_unquotes(self, value):
    HEAP.set_field(self, ImageQuoteExpression_UnquotesOffset, value)

class ImageUnquoteExpression(ImageSyntaxTree):
  def __init__(self, addr, index):
    ImageSyntaxTree.__init__(self, addr)
    self.set_index(index)
  def set_index(self, value):
    HEAP.set_raw_field(self, ImageUnquoteExpression_IndexOffset, value)

# -----------------------
# --- C o m p i l e r ---
# -----------------------

PLACEHOLDER = 0xBADDEAD

class GlobalScope:
  def lookup(self, name):
    return Global(name)

class LocalScope:
  def __init__(self, parent, symbols):
    self.symbols = symbols
    self.parent = parent
  def lookup(self, name):
    for symbol in self.symbols:
      if symbol.name == name:
        return Local(symbol)
    return self.parent.lookup(name)

def serialize_value(value):
  if type(value) is str:
    return '"' + value + '"'
  else:
    return str(value)

def compile(str):
  scanner = Scanner(str)
  parser = Parser(scanner)
  scanner.set_parser(parser)
  return parser.parse_program()


# -----------------------
# --- V i s i t o r s ---
# -----------------------

class Visitor:
  def visit_node(self, that):
    that.traverse(self)
  def visit_program(self, that):
    self.visit_node(that)
  def visit_definition(self, that):
    self.visit_node(that)
  def visit_builtin_class(self, that):
    self.visit_node(that)
  def visit_lambda(self, that):
    self.visit_node(that)
  def visit_sequence(self, that):
    self.visit_node(that)
  def visit_call(self, that):
    self.visit_node(that)
  def visit_this(self, that):
    self.visit_node(that)
  def visit_void(self, that):
    self.visit_node(that)
  def visit_true(self, that):
    self.visit_node(that)
  def visit_false(self, that):
    self.visit_node(that)
  def visit_null(self, that):
    self.visit_node(that)
  def visit_literal(self, that):
    self.visit_node(that)
  def visit_identifier(self, that):
    self.visit_node(that)
  def visit_return(self, that):
    self.visit_node(that)
  def visit_invoke(self, that):
    self.visit_node(that)
  def visit_method(self, that):
    self.visit_node(that)
  def visit_internal_call(self, that):
    self.visit_node(that)
  def visit_conditional(self, that):
    self.visit_node(that)
  def visit_quote(self, that):
    self.visit_node(that)
  def visit_unquote(self, that):
    self.visit_node(that)
  def visit_tuple(self, that):
    self.visit_node(that)
  def visit_builtin_lambda(self, that):
    self.visit_node(that)
  def visit_interpolated(self, that):
    self.visit_node(that)
  def visit_local_definition(self, that):
    self.visit_node(that)
  def visit_raise(self, that):
    self.visit_node(that)
  def visit_do_on_expression(self, that):
    self.visit_node(that)
  def visit_on_clause(self, that):
    self.visit_node(that)
  def visit_instantiate(self, that):
    self.visit_node(that)
  def visit_protocol(self, that):
    self.visit_node(that)
  def visit_arguments(self, that):
    self.visit_node(that)
  def visit_external_call(self, that):
    self.visit_node(that)
  def visit_task(self, that):
    self.visit_node(that)
  def visit_yield(self, that):
    self.visit_node(that)

class LoadVisitor(Visitor):
  def __init__(self):
    self.is_toplevel = True
  def do_it(self, that, name, name_str):
    protocol = HEAP.new_protocol(name_str)
    static_protocol = HEAP.new_protocol(HEAP.void)
    that.set_image(protocol)
    that.static_protocol = static_protocol
    HEAP.toplevel[name_str] = protocol
    that.image = protocol
    if self.is_toplevel:
      NAMESPACE[name] = that
  def visit_protocol(self, that):
    name_str = HEAP.new_string(that.name)
    self.do_it(that, that.name, name_str)
    was_toplevel = self.is_toplevel
    self.is_toplevel = False
    Visitor.visit_protocol(self, that)
    self.is_toplevel = was_toplevel
  def visit_builtin_class(self, that):
    instance_type_name = that.info.instance_type
    instance_type_index = globals()[instance_type_name + '_TYPE']
    name_str = HEAP.new_string(that.info.class_name)
    self.do_it(that, that.info.class_name, name_str)
    HEAP.set_root(that.info.root_index, that.image)
    assert self.is_toplevel
    self.is_toplevel = False
    Visitor.visit_builtin_class(self, that)
    self.is_toplevel = True

class ResolveVisitor(Visitor):
  def resolve_class(self, that):
    if that.super_name:
      value = NAMESPACE[that.super_name]
      that.image.set_super(value.image)
      that.super = value
    else:
      that.image.set_super(HEAP.void)
      that.super = None
  def visit_builtin_class(self, that):
    self.resolve_class(that)
    Visitor.visit_builtin_class(self, that)
  def visit_protocol(self, that):
    self.resolve_class(that)
    Visitor.visit_protocol(self, that)

class DummySignatureProvider(SignatureProvider):
  def __init__(self, protocol):
    SignatureProvider.__init__(self)
    self.protocol = protocol
  def get_image(self):
    return self.protocol

class CompileVisitor(Visitor):
  def compile_methods(self, that):
    methods = HEAP.new_tuple(values = [ m.compile(that) for m in that.members if not m.is_static ])
    provider = DummySignatureProvider(that.static_protocol)
    statics = HEAP.new_tuple(values = [ m.compile(provider) for m in that.members if m.is_static ])
    that.static_protocol.set_methods(statics)
    if that.super_name:
      that.static_protocol.set_super(that.super.static_protocol)
    else:
      that.static_protocol.set_super(NAMESPACE['Protocol'].image)
    layout = HEAP.new_layout(that.static_protocol, HEAP.empty_tuple, PROTOCOL_TYPE, 0)
    that.image.set_methods(methods)
    that.image.set_layout(layout)
  def visit_protocol(self, that):
    self.compile_methods(that)
    Visitor.visit_protocol(self, that)
  def visit_builtin_class(self, that):
    self.compile_methods(that)
    Visitor.visit_builtin_class(self, that)
  def visit_definition(self, that):
    value = that.value.compile()
    HEAP.toplevel[HEAP.new_string(that.name)] = value
    Visitor.visit_definition(self, that)


# ---------------
# --- M a i n ---
# ---------------

# The set of command-line options
options = [
  Option('-o', '--output', dest='out', help='output file', nargs=1),
  Option('-c', '--consts', dest='consts', help='constants', nargs=1)
]

# Validates the parsed command-line arguments
def process_args(values, files):
  if not values.out:
    print "An output file must be specified."
    return False
  if not values.consts:
    print "A constants file must be specified."
    return False
  for file in files:
    if not path.exists(file):
      print "Could not find file " + file + "."
      return False
  return True

class MacroCollection:
  def __init__(self):
    self.defines = { }
  def add(self, name, value):
    self.defines[name] = value
  def apply(self, name, function):
    macro = self.defines[name]
    macro.apply(self, function)

class Macro:
  def __init__(self, args, body):
    self.args = args
    self.body = body
  def apply(self, macros, function):
    for arg in self.args:
      call_pattern = re.compile(arg + "\(([^)]*)\)")
      calls = call_pattern.findall(self.body)
      for call in calls:
        args = map(string.strip, call.split(","))
        function(*args)
    forward_pattern = re.compile("([A-Za-z0-9_]+)\(" + ", ".join(self.args) + '\)')
    forwards = forward_pattern.findall(self.body)
    for forward in forwards:
      macros.apply(forward, function)

const_pattern = re.compile("^#define ([a-zA-Z0-9_]+)\(([^)]*)\)(.*)$")
def read_consts(file):
  defines = MacroCollection()
  input = open(file).read()
  input = input.replace("\\\n", "")
  lines = input.split("\n")
  for line in lines:
    stripped = line.strip()
    match = const_pattern.match(stripped)
    if not match: continue
    name = match.group(1).strip()
    args = match.group(2).strip().split()
    body = match.group(3).strip()
    defines.add(name, Macro(args, body))
  return defines

def define_type_tag(n, TYPE, Type, info):
  globals()[TYPE + '_TYPE'] = int(n)

class BuiltinClassInfo:
  def __init__(self, root_index, name, instance_type, class_name):
    self.root_index = root_index
    self.name = name
    self.instance_type = instance_type
    self.class_name = class_name

ROOT_INDEX = { }
def define_root(n, Class, name, Name, allocator):
  assert Name not in ROOT_INDEX
  ROOT_INDEX[Name] = int(n)

def define_image_object_const(n, Type, Name):
  name = 'Image' + Type + '_' + Name
  globals()[name] = int(n)

BUILTIN_CLASSES = { }
CLASSES_BY_ROOT_NAME = { }
def define_builtin_class(Class, name, NAME):
  root_name = Class
  if root_name in ROOT_INDEX: root_index = ROOT_INDEX[root_name]
  else: root_index = -1
  info = BuiltinClassInfo(root_index, name, NAME, Class)
  BUILTIN_CLASSES[Class] = info
  CLASSES_BY_ROOT_NAME[name] = info

BUILTIN_METHODS = { }
def define_builtin_method(n, root, name, string):
  string_name = string[1:-1]
  Class = CLASSES_BY_ROOT_NAME[root].class_name
  BUILTIN_METHODS[(Class, string_name)] = int(n)

BUILTIN_FUNCTIONS = { }
def define_builtin_function(n, name, string):
  string_name = string[1:-1]
  BUILTIN_FUNCTIONS[string_name] = int(n)

# This function imports a set of constants from a C++ header file, by
# expanding macros into the global python namespace.  Slightly cheesy
# but very convenient indeed.
def import_constants(file):
  consts = read_consts(file)
  consts.apply('FOR_EACH_DECLARED_TYPE', define_type_tag)
  consts.apply('FOR_EACH_ROOT', define_root)
  consts.apply('FOR_EACH_BUILTIN_LAYOUT', define_builtin_class)
  consts.apply('FOR_EACH_IMAGE_OBJECT_CONST', define_image_object_const)
  consts.apply('FOR_EACH_BUILTIN_METHOD', define_builtin_method)
  consts.apply('FOR_EACH_BUILTIN_FUNCTION', define_builtin_function)

# Returns a list of all source files in the given directory
def find_source_files(dir):
  result = []
  def visit(result, dirname, fnames):
    if path.basename(dirname).startswith('.'): return
    for fname in fnames:
      if fname.endswith('.n'):
        result.append(path.join(dirname, fname))
  path.walk(dir, visit, result)
  return result

NAMESPACE = { }

def load_files(files):
  trees = []
  for file in files:
    if path.isdir(file):
      files += find_source_files(file)
    else:
      source = codecs.open(file, "r", "utf-8").read()
      tree = compile(source)
      trees.append(tree)
  for tree in trees: tree.accept(LoadVisitor())
  for tree in trees: tree.accept(ResolveVisitor())
  for tree in trees: tree.accept(CompileVisitor())

def main():
  parser = OptionParser(option_list=options)
  (values, files) = parser.parse_args()
  if not process_args(values, files):
    parser.print_help()
    return
  import_constants(values.consts)
  global HEAP
  HEAP = Heap()
  HEAP.initialize()
  load_files(files)
  HEAP.write_to(values.out)

if __name__ == '__main__':
  main()
