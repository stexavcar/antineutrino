# -*- coding: utf-8 -*-

import ast
import reader
import scanner


# Install the keywords as global variables, so 'this' can be accessed
# through THIS and so forth.
for word in scanner.KEYWORDS.keys() + scanner.MODIFIERS.keys():
  globals()[word.upper()] = word


class Parser(object):

  def __init__(self, scan):
    self.scan_ = scan
    self.resolver_ = VariableResolver()
    self.quoter_ = QuoteLevel(None)
    scan.set_parser(self)

  def scanner(self):
    return self.scan_

  def resolver(self):
    return self.resolver_

  def quoter(self):
    return self.quoter_

  def token(self):
    return self.scanner().current()

  def advance(self):
    self.scanner().advance()

  def has_more(self):
    return self.scanner().has_more()

  def open(doc):
    read = reader.Reader(doc)
    scan = scanner.Scanner(read)
    result = Parser(scan)
    scan.advance()
    return result
  open = staticmethod(open)

  def expect_documentation(self):
    if not self.token().is_documentation():
      self.unexpected_token()
    value = self.token().value()
    self.advance()
    return value

  def expect_identifier(self):
    if not self.token().is_identifier():
      self.unexpected_token()
    value = self.token().value()
    self.advance()
    return value

  def expect_string(self):
    if not self.token().is_string():
      self.unexpected_token()
    value = self.token().value()
    self.advance()
    return value

  def expect_number(self):
    if not self.token().is_number():
      self.unexpected_token()
    value = self.token().value()
    self.advance()
    return value

  def expect_keyword(self, value = None):
    if not self.token().is_keyword(value):
      self.unexpected_token()
    value = self.token().value()
    self.advance()
    return value

  def expect_delimiter(self, value):
    if not self.token().is_delimiter(value):
      self.unexpected_token()
    self.advance()

  def expect_operator(self, value = None):
    if not self.token().is_operator(value):
      self.unexpected_token()
    value = self.token().value()
    self.advance()
    return value

  def unexpected_token(self):
    pos = self.token().position()
    raise scanner.SyntaxError(pos, 'Unexpected token')

  def at_modifier(self, allow_idents):
    if allow_idents and self.token().is_identifier():
      return True
    return self.token().is_delimiter('@')

  def modifier(self, allow_idents):
    if allow_idents and self.token().is_identifier():
      return self.expect_identifier()
    else:
      self.expect_delimiter('@')
      return self.expect_identifier();

  def modifiers(self, allow_idents):
    result = [ ]
    while self.at_modifier(allow_idents):
      modifier = self.modifier(allow_idents)
      result.append(modifier)
    return result

  def program(self):
    definitions = [ ]
    while self.has_more():
      definition = self.toplevel_definition()
      definitions.append(definition)
    return ast.File(definitions)

  def toplevel_definition(self):
    doc = self.documentation_opt()
    modifiers = self.modifiers(True)
    if self.token().is_keyword(DEF):
      return self.definition(modifiers)
    elif self.token().is_keyword(PROTOCOL):
      protocol = self.protocol(modifiers)
      return ast.Definition(modifiers, protocol.name(), protocol)
    else:
      self.unexpected_token()

  def definition(self, modifiers):
    self.expect_keyword(DEF)
    name = self.expect_identifier()
    if self.token().is_delimiter(';'):
      self.expect_delimiter(';')
      return ast.Definition(modifiers, name, None)
    elif self.token().is_delimiter(':='):
      self.expect_delimiter(':=')
      value = self.expression(False)
      self.expect_delimiter(';')
      return ast.Definition(modifiers, name, value)
    else:
      params = self.parameters()
      self.resolver().push_scope(params.params())
      body = self.function_body(True)
      self.resolver().pop_scope()
      return ast.Definition(modifiers, name, ast.Lambda(modifiers, name, params, body, False))

  def protocol(self, modifiers):
    self.expect_keyword(PROTOCOL)
    name = self.expect_identifier()
    if self.token().is_delimiter(':'):
      self.expect_delimiter(':')
      parent = self.expect_identifier()
    else:
      parent = None
    self.expect_delimiter('{')
    members = [ ]
    while not self.token().is_delimiter('}'):
      member = self.member_declaration(name)
      members.append(member)
    self.expect_delimiter('}')
    return ast.Protocol(modifiers, name, parent, members)

  def member_declaration(self, klass):
    doc = self.documentation_opt()
    modifiers = self.modifiers(True)
    self.expect_keyword(DEF)
    (name, params) = self.method_name()
    if params is None:
      if self.token().is_delimiter('('):
        params = self.parameters()
      else:
        params = ast.Parameters(0, [], True)
    self.resolver().push_scope(params.params())
    body = self.function_body(True)
    self.resolver().pop_scope()
    return ast.Method(doc, modifiers, klass, name, params, body)

  def method_name(self):
    if self.token().is_identifier():
      return (self.expect_identifier(), None)
    elif self.token().is_delimiter('['):
      params = self.parameters('[', ']')
      if self.token().is_delimiter(':='):
        self.expect_delimiter(':=')
        set_params = self.parameters()
        return ("[]:=", params.extend(set_params))
      else:
        return ("[]", params)
    elif self.token().is_operator():
      value = self.expect_operator()
      if scanner.is_circumfix_operator(value):
        match = self.expect_operator(scanner.circumfix_match(value))
        return (value + match, ast.Parameters(0, [], False))
      return (value, None)
    elif self.token().is_keyword():
      return (self.expect_keyword(), None)
    else:
      self.unexpected_token()

  def function_body(self, is_toplevel):
    if self.token().is_delimiter(';'):
      self.expect_delimiter(';')
      return None
    elif self.token().is_delimiter('->'):
      self.expect_delimiter('->')
      value = self.expression(False)
      if is_toplevel: self.expect_delimiter(';')
      return ast.Return(value)
    elif self.token().is_delimiter('{'):
      self.expect_delimiter('{')
      exprs = self.statements()
      self.expect_delimiter('}')
      exprs.append(ast.Return(ast.Literal(ast.Void())))
      return ast.Sequence.make(exprs)
    else:
      self.unexpected_token()

  def statements(self):
    stmts = [ ]
    while self.has_more() and not self.token().is_delimiter('}'):
      stmt = self.expression(True)
      stmts.append(stmt)
    return stmts

  def parameters(self, start = '(', end = ')'):
    params = [ ]
    keywords = [ ]
    self.expect_delimiter(start)
    if not self.token().is_delimiter(end):
      self.parameter(params, keywords)
      while self.token().is_delimiter(','):
        self.expect_delimiter(',')
        self.parameter(params, keywords)
    self.expect_delimiter(end)
    all_params = params + sorted(keywords, key=ast.Symbol.name)
    return ast.Parameters(len(params), all_params, False)

  def parameter(self, params, keywords):
    if self.token().is_delimiter(u'‹'):
      params.append(self.unquote_expression())
    else:
      name = self.expect_identifier()
      symbol = ast.Symbol(name)
      if self.token().is_delimiter(':'):
        self.expect_delimiter(':')
        keywords.append(symbol)
      else:
        params.append(symbol)

  def expression(self, is_toplevel):
    return self.annotated_expression(is_toplevel)
  
  def annotated_expression(self, is_toplevel):
    modifiers = self.modifiers(False)
    return self.control_expression(modifiers, is_toplevel)

  def control_expression(self, modifiers, is_toplevel):
    if self.token().is_keyword([DEF, VAR]):
      return self.local_definition(modifiers, is_toplevel)
    elif self.token().is_keyword(REC):
      return self.recursive_definition(is_toplevel)
    elif self.token().is_keyword(IF):
      return self.conditional_expression(is_toplevel)
    elif self.token().is_keyword(WHILE):
      return self.while_expression(is_toplevel)
    elif self.token().is_keyword(RETURN):
      return self.return_expression(is_toplevel)
    elif self.token().is_keyword(YIELD):
      return self.yield_expression(is_toplevel)
    elif self.token().is_keyword(DO):
      return self.do_on_expression(is_toplevel)
    elif self.token().is_keyword(RAISE):
      return self.raise_expression(is_toplevel)
    elif self.token().is_keyword(ASSERT):
      return self.assert_expression(is_toplevel)
    elif self.token().is_delimiter('{'):
      return self.sequence_expression()
    else:
      value = self.lambda_expression(modifiers)
      if is_toplevel: self.expect_delimiter(';')
      return value

  def do_on_expression(self, is_toplevel):
    self.expect_keyword(DO)
    value = self.expression(is_toplevel)
    clauses = [ ]
    while self.token().is_keyword('on'):
      self.expect_keyword(ON)
      name = self.expect_identifier()
      params = self.parameters()
      body = self.function_body(is_toplevel)
      clauses.append(ast.OnClause(name, ast.Lambda([], None, params, ast.Return(body), False)))
    return ast.DoOnExpression(value, clauses)

  def lambda_expression(self, modifiers):
    if self.token().is_keyword(FN):
      self.expect_keyword(FN)
      params = self.parameters()
      self.resolver().push_scope(params.params())
      body = self.function_body(False)
      self.resolver().pop_scope()
      if 'local' in modifiers:
        return ast.Lambda([], None, params, body, True)
      else:
        return ast.Lambda([], None, params, body, False)
    if self.token().is_keyword(TASK):
      self.expect_keyword(TASK)
      params = ast.Parameters(0, [], False)
      self.resolver().push_scope(params.params())
      body = self.function_body(False)
      self.resolver().pop_scope()
      return ast.Task(ast.Lambda([], None, params, body, False))
    else:
      return self.logical_expression()

  def yield_expression(self, is_toplevel):
    self.expect_keyword(YIELD)
    value = self.expression(False)
    if is_toplevel: self.expect_delimiter(';')
    return ast.Yield(value)

  def return_expression(self, is_toplevel):
    self.expect_keyword(RETURN)
    value = self.expression(False)
    if is_toplevel: self.expect_delimiter(';')
    return ast.Return(value)
  
  def assert_expression(self, is_toplevel):
    self.expect_keyword(ASSERT)
    value = self.expression(False)
    if is_toplevel: self.expect_delimiter(';')
    if value.is_invoke():
      defs = [ ]
      recv_sym = ast.Symbol("")
      raise_args = [ ]
      defs.append((recv_sym, value.recv()))
      raise_args.append(recv_sym)
      name = value.name()
      args = value.args()
      argsyms = [ ]
      for arg in args.arguments():
        argsym = ast.Symbol("")
        argsyms.append(argsym)
        defs.append((argsym, arg))
        raise_args.append(argsym)
      condition = ast.Invoke(recv_sym, name, args.clone_with_args(argsyms))
      error = ast.Raise("assertion_failure", ast.Arguments(raise_args, {}, False))
      result = ast.Conditional(condition, ast.Literal(ast.Void()), error)
      defs.reverse()
      for (sym, value) in defs:
        result = ast.LocalDefinition(sym, value, result, 1)
      return result
    else:
      error = ast.Raise("assertion_failure", ast.Arguments([], {}, False))
      return ast.Conditional(value, ast.Literal(ast.Void()), error)

  def raise_expression(self, is_toplevel):
    self.expect_keyword(RAISE)
    name = self.expect_identifier()
    args = self.arguments()
    if is_toplevel: self.expect_delimiter(';')
    return ast.Raise(name, args)

  def conditional_expression(self, is_toplevel):
    self.expect_keyword(IF)
    self.expect_delimiter('(')
    cond = self.expression(False)
    self.expect_delimiter(')')
    then_part = self.expression(is_toplevel)
    if self.token().is_keyword(ELSE):
      self.expect_keyword(ELSE)
      else_part = self.expression(is_toplevel)
    else:
      else_part = ast.Literal(ast.Void())
    return ast.Conditional(cond, then_part, else_part)
  
  def while_expression(self, is_toplevel):
    self.expect_keyword(WHILE)
    self.expect_delimiter('(')
    cond = self.expression(False)
    self.expect_delimiter(')')
    body = self.expression(is_toplevel)
    return ast.While(cond, body)
  
  def sequence_expression(self):
    self.expect_delimiter('{')
    result = self.statements()
    self.expect_delimiter('}')
    return ast.Sequence.make(result)

  def local_definition(self, modifiers, is_toplevel):
    is_var = self.token().is_keyword(VAR)
    if is_var: self.expect_keyword(VAR)
    else: self.expect_keyword(DEF)
    name = ast.Symbol(self.expect_identifier())
    self.expect_delimiter(':=')
    value = self.expression(False)
    self.resolver().push_scope([ name ])
    if self.token().is_keyword(IN):
      self.expect_keyword(IN)
      body = self.expression(is_toplevel)
      if is_toplevel: self.expect_delimiter(';')
    elif is_toplevel:
      self.expect_delimiter(';')
      stmts = self.statements()
      body = ast.Sequence.make(stmts)
    self.resolver().pop_scope()
    if 'local' in modifiers:
      type = 4
    else:
      if is_var: type = 2
      else: type = 1
    return ast.LocalDefinition(name, value, body, type)

  def recursive_definition(self, is_toplevel):
    self.expect_keyword(REC)
    name = ast.Symbol(self.expect_identifier())
    self.resolver().push_scope([ name ])
    self.expect_delimiter(':=')
    value = self.expression(False)
    if self.token().is_keyword(IN):
      self.expect_keyword(IN)
      body = self.expression(is_toplevel)
    elif is_toplevel:
      self.expect_delimiter(';')
      stmts = self.statements()
      body = ast.Sequence.make(stmts)
    self.resolver().pop_scope()
    return ast.LocalDefinition(name, value, body, 3)

  def logical_expression(self):
    exprs = [ self.and_expression() ]
    while self.token().is_keyword(OR):
      self.expect_keyword(OR)
      exprs.append(self.and_expression())
    result = exprs[-1]
    for i in xrange(2, len(exprs) + 1):
      result = ast.Conditional(exprs[-i], ast.Literal(ast.Thrue()), result)
    return result

  def and_expression(self):
    exprs = [ self.not_expression() ]
    while self.token().is_keyword(AND):
      self.expect_keyword(AND)
      exprs.append(self.not_expression())
    # We construct the resulting conditional expression backwards
    # To make the conditionals simple
    result = exprs[-1];
    for i in xrange(2, len(exprs) + 1):
      result = ast.Conditional(exprs[-i], result, ast.Literal(ast.Fahlse()))
    return result

  def not_expression(self):
    if self.token().is_keyword(NOT):
      self.expect_keyword(NOT)
      expr = self.not_expression()
      return ast.Conditional(expr, ast.Literal(ast.Fahlse()), ast.Literal(ast.Thrue()))
    else:
      return self.assignment_expression()

  def assignment_expression(self):
    value = self.operator_expression(None)
    if self.token().is_delimiter(':='):
      if value.is_identifier():
        local = self.resolver().lookup(value.name())
        symbol = local.symbol()
        self.expect_delimiter(':=')
        rvalue = self.assignment_expression()
        return ast.Assignment(symbol, rvalue)
      elif value.is_invoke():
        recv = value.recv()
        name = value.name() + ':='
        args = value.args()
        self.expect_delimiter(':=')
        rvalue = self.assignment_expression()
        return ast.Invoke(recv, name, args.extend(rvalue))
      else:
        self.unexpected_token()
    else:
      return value
 
  def circumfix_expression(self):
    assert self.token().is_operator()
    value = self.token().value()
    if not scanner.is_circumfix_operator(value):
      self.unexpected_token()
    self.expect_operator()
    match = scanner.circumfix_match(value)
    expr = self.operator_expression(match)
    self.expect_operator(match)
    return ast.Invoke(expr, value + match, ast.Arguments([], {}, False))

  def operator_expression(self, match):
    exprs = [ self.call_expression() ]
    ops = [ ]
    while self.token().is_operator() and (not match or not self.token().is_operator(match)):
      ops.append(self.expect_operator())
      exprs.append(self.call_expression())
    # TODO: operator precedence
    while ops:
      rest = exprs.pop()
      next = exprs.pop()
      op = ops.pop()
      args = ast.Arguments([rest], {}, False)
      exprs.append(ast.Invoke(next, op, args))
    return exprs[0]

  def at_call_start(self):
    token = self.token()
    return (token.is_delimiter(scanner.DELEGATE_CALL_OPERATOR)
         or token.is_delimiter('.')
         or token.is_delimiter('(')
         or token.is_delimiter('['))

  def call_expression(self):
    expr = self.unary_expression()
    while self.at_call_start():
      start_token = self.token()
      if self.token().is_delimiter('.'):
        self.expect_delimiter('.')
        name = self.expect_identifier()
        if self.token().is_delimiter('('):
          args = self.arguments()
        else:
          args = ast.Arguments([], {}, True)
        expr = self.call_prefix(expr, name, args)
      elif self.token().is_delimiter(scanner.DELEGATE_CALL_OPERATOR):
        self.expect_delimiter(scanner.DELEGATE_CALL_OPERATOR)
        fun = self.atomic_expression()
        args = self.arguments()
        expr = ast.Call(expr, fun, args)
      elif self.token().is_delimiter('['):
        args = self.arguments('[', ']')
        expr = self.call_prefix(expr, "[]", args)
      else:
        args = self.arguments()
        expr = ast.Call(ast.This(), expr, args)
    return expr

  def new_expression(self):
    self.expect_keyword(NEW)
    if self.token().is_delimiter('{'):
      recv = ast.Global('Object')
      args = ast.Arguments([], {}, False)
    else:
      recv = self.atomic_expression()
      args = self.arguments()
    return self.call_prefix(recv, "new", args)

  def call_prefix(self, recv, name, args):
    if self.token().is_delimiter('{'):
      self.expect_delimiter('{')
      terms = [ ]
      while not self.token().is_delimiter('}'):
        name = self.expect_identifier()
        self.expect_delimiter(':')
        value = self.expression(False)
        terms.append((name, value))
        if not self.token().is_delimiter(','): break
        else: self.expect_delimiter(',')
      self.expect_delimiter('}')
      return ast.Instantiate(recv, name, args, terms)
    else:
      return ast.Invoke(recv, name, args)

  def unary_expression(self):
    return self.atomic_expression()

  def atomic_expression(self):
    if self.token().is_keyword(THIS):
      self.expect_keyword(THIS)
      return ast.This()
    if self.token().is_keyword(SUPER):
      self.expect_keyword(SUPER)
      return ast.Super(ast.This())
    elif self.token().is_keyword(NULL):
      self.expect_keyword(NULL)
      return ast.Literal(ast.Null())
    elif self.token().is_keyword(TRUE):
      self.expect_keyword(TRUE)
      return ast.Literal(ast.Thrue())
    elif self.token().is_keyword(FALSE):
      self.expect_keyword(FALSE)
      return ast.Literal(ast.Fahlse())
    elif self.token().is_keyword(PROTOCOL):
      return self.protocol([])
    elif self.token().is_string():
      terms = self.expect_string()
      if len(terms) == 0:
        return ast.Literal("")
      elif len(terms) == 1 and not isinstance(terms[0], ast.SyntaxTree):
        return ast.Literal(terms[0])
      else:
        return ast.Interpolate(terms)
    elif self.token().is_number():
      value = self.expect_number()
      return ast.Literal(value)
    elif self.token().is_identifier():
      name = self.expect_identifier()
      return self.resolver().lookup(name)
    elif self.token().is_keyword(NEW):
      return self.new_expression()
    elif self.token().is_delimiter('('):
      self.expect_delimiter('(')
      value = self.expression(False)
      self.expect_delimiter(')')
      return value
    elif self.token().is_delimiter(u'«'):
      return self.quote_expression()
    elif self.token().is_delimiter(u'‹'):
      return self.unquote_expression()
    elif self.token().is_operator():
      return self.circumfix_expression()
    elif self.token().is_delimiter('['):
      return self.tuple()
    else:
      self.unexpected_token()

  def quote_expression(self):
    self.expect_delimiter(u'«')
    my_quoter = QuoteLevel(self.quoter())
    self.quoter_ = my_quoter
    value = self.expression(False)
    self.expect_delimiter(u'»')
    self.quoter_ = my_quoter.parent()
    unquotes = my_quoter.unquotes()
    return ast.Quote(value, unquotes)
    return value

  def unquote_expression(self):
    self.expect_delimiter(u'‹')
    current_quoter = self.quoter()
    self.quoter_ = current_quoter.parent()
    expr = self.expression(False)
    self.quoter_ = current_quoter
    self.expect_delimiter(u'›')
    index = current_quoter.register_unquoted(expr)
    return ast.Unquote(index)

  def tuple(self):
    self.expect_delimiter('[')
    elms = [ ]
    if not self.token().is_delimiter(']'):
      elms.append(self.expression(False))
    while self.token().is_delimiter(','):
      self.expect_delimiter(',')
      elms.append(self.expression(False))
    self.expect_delimiter(']')
    return ast.Tuple(elms)

  def arguments(self, start='(', end=')'):
    self.expect_delimiter(start)
    args = [ ]
    keywords = { }
    def parse_argument():
      expr = self.expression(False)
      if self.token().is_delimiter(':') and expr.is_identifier():
        self.expect_delimiter(':')
        keywords[expr.name()] = len(args)
        expr = self.expression(False)
      args.append(expr)
    if not self.token().is_delimiter(end):
      parse_argument()
    while self.token().is_delimiter(','):
      self.advance()
      parse_argument()
    self.expect_delimiter(end)
    return ast.Arguments(args, keywords, False)

  def documentation_opt(self):
    if self.token().is_documentation():
      doc = self.expect_documentation()
      return ast.Documentation(doc)
    else:
      return None


# --- S c o p e s ---


class GlobalScope(object):

  def lookup(self, name):
    return ast.Global(name)


class LocalScope(object):

  def __init__(self, parent, symbols):
    self.symbols_ = symbols
    self.parent_ = parent

  def parent(self):
    return self.parent_

  def lookup(self, name):
    for symbol in self.symbols_:
      if symbol.name() == name:
        return ast.Local(symbol)
    return self.parent_.lookup(name)


class VariableResolver(object):

  def __init__(self):
    self.scope_ = GlobalScope()

  def scope(self):
    return self.scope_

  def lookup(self, name):
    return self.scope().lookup(name)

  def push_scope(self, symbols):
    entries = [ ]
    for entry in symbols:
      if isinstance(entry, ast.Symbol):
        entries.append(entry)
    self.scope_ = LocalScope(self.scope_, entries)

  def pop_scope(self):
    self.scope_ = self.scope().parent()


# --- Q u o t i n g ---


class QuoteLevel(object):

  def __init__(self, parent):
    self.parent_ = parent
    self.unquotes_ = [ ]

  def parent(self):
    return self.parent_

  def unquotes(self):
    return self.unquotes_

  def register_unquoted(self, expr):
    index = len(self.unquotes_)
    self.unquotes_.append(expr)
    return index
