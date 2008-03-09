import ast
import reader
import scanner


# Install the keywords as global variables, so 'this' can be accessed
# through THIS and so forth.
for word in scanner.KEYWORDS.keys():
  globals()[word.upper()] = word


MODIFIERS = [ INTERNAL, NATIVE ]


class Parser(object):

  def __init__(self, scan):
    self.scan_ = scan
    self.resolver_ = VariableResolver()

  def scanner(self):
    return self.scan_

  def resolver(self):
    return self.resolver_

  def token(self):
    return self.scanner().current()

  def advance(self):
    self.scanner().advance()

  def has_more(self):
    return self.scanner().has_more()

  def open(doc):
    read = reader.Reader(doc)
    scan = scanner.Scanner(read)
    return Parser(scan)
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

  def is_modifier(self, token):
    for modifier in MODIFIERS:
      if token.is_keyword(modifier):
        return True
    return False

  def modifiers(self):
    result = [ ]
    while self.is_modifier(self.token()):
      modifier = self.token().value()
      result.append(modifier)
      self.expect_keyword(modifier)
    return result

  def program(self):
    definitions = [ ]
    while self.has_more():
      definition = self.toplevel_definition()
      definitions.append(definition)
    return ast.File(definitions)

  def toplevel_definition(self):
    doc = self.documentation_opt()
    modifiers = self.modifiers()
    if self.token().is_keyword(DEF):
      return self.definition(modifiers)
    elif self.token().is_keyword(PROTOCOL):
      return self.protocol_declaration(modifiers)
    else:
      self.unexpected_token()

  def definition(self, modifiers):
    self.expect_keyword(DEF)
    name = self.expect_identifier()
    params = self.parameters()
    self.resolver().push_scope(params)
    body = self.function_body(True)
    self.resolver().pop_scope()
    return ast.Definition(name, ast.Lambda(modifiers, name, params, body))

  def protocol_declaration(self, modifiers):
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
    protocol = ast.Protocol(modifiers, name, parent, members)
    return ast.Definition(name, protocol)

  def member_declaration(self, klass):
    doc = self.documentation_opt()
    modifiers = self.modifiers()
    self.expect_keyword(DEF)
    name = self.method_name()
    if self.token().is_delimiter('('):
      params = self.parameters()
      has_params = True
    else:
      params = ast.Null()
      has_params = False
    if has_params: self.resolver().push_scope(params)
    body = self.function_body(True)
    if has_params: self.resolver().pop_scope()
    return ast.Method(doc, modifiers, klass, name, params, body)

  def method_name(self):
    if self.token().is_identifier():
      return self.expect_identifier()
    elif self.token().is_operator():
      value = self.expect_operator()
      if scanner.is_circumfix_operator(value):
        match = self.expect_operator(scanner.circumfix_match(value))
        return value + match
      return value
    elif self.token().is_keyword():
      return self.expect_keyword()
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
      return ast.Sequence(exprs)
    else:
      self.unexpected_token()

  def statements(self):
    stmts = [ ]
    while self.has_more() and not self.token().is_delimiter('}'):
      stmt = self.control_expression(True)
      stmts.append(stmt)
    return stmts

  def parameters(self):
    result = [ ]
    self.expect_delimiter('(')
    if not self.token().is_delimiter(')'):
      result.append(self.parameter())
      while self.token().is_delimiter(','):
        self.expect_delimiter(',')
        result.append(self.parameter())
    self.expect_delimiter(')')
    return result

  def parameter(self):
    if self.token().is_delimiter('~'):
      return self.unquote_expression()
    else:
      name = self.expect_identifier()
      if self.token().is_delimiter(':'):
        self.expect_delimiter(':')
      return ast.Symbol(name)

  def expression(self, is_toplevel):
    return self.control_expression(is_toplevel)

  def control_expression(self, is_toplevel):
    if self.token().is_keyword(DEF):
      return self.local_definition(is_toplevel)
    elif self.token().is_keyword(IF):
      return self.conditional_expression(is_toplevel)
    elif self.token().is_keyword(RETURN):
      return self.return_expression(is_toplevel)
    elif self.token().is_keyword(RAISE):
      return self.raise_expression(is_toplevel)
    else:
      value = self.logical_expression()
      if is_toplevel: self.expect_delimiter(';')
      return value

  def return_expression(self, is_toplevel):
    self.expect_keyword(RETURN)
    value = self.expression(False)
    if is_toplevel: self.expect_delimiter(';')
    return ast.Return(value)

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

  def local_definition(self, is_toplevel):
    self.expect_keyword(DEF)
    name = ast.Symbol(self.expect_identifier())
    self.expect_delimiter(':=')
    value = self.expression(False)
    self.resolver().push_scope([ name ])
    if self.token().is_keyword(IN):
      self.expect_keyword(IN)
      body = self.expression(is_toplevel)
    elif is_toplevel:
      self.expect_delimiter(';')
      stmts = self.statements()
      body = ast.Sequence.make(stmts)
    self.resolver().pop_scope()
    return ast.LocalDefinition(name, value, body)

  def logical_expression(self):
    return self.and_expression()

  def and_expression(self):
    return self.not_expression()

  def not_expression(self):
    return self.prefix_expression()

  def prefix_expression(self):
    return self.operator_expression()

  def operator_expression(self):
    exprs = [ self.call_expression() ]
    ops = [ ]
    while self.token().is_operator():
      ops.append(self.expect_operator())
      exprs.append(self.call_expression())
    # TODO: operator precedence
    if ops:
      exprs.reverse()
      ops.reverse()
      while ops:
        rest = exprs.pop()
        next = exprs.pop()
        op = ops.pop()
        args = ast.Arguments([rest], {})
        exprs.append(ast.Invoke(next, op, args))
    return exprs[0]

  def at_call_start(self):
    token = self.token()
    return (token.is_delimiter(scanner.DELEGATE_CALL_OPERATOR)
         or token.is_delimiter(scanner.METHOD_CALL_OPERATOR)
         or token.is_delimiter('('))

  def call_expression(self):
    expr = self.unary_expression()
    while self.at_call_start():
      start_token = self.token()
      if self.token().is_delimiter(scanner.METHOD_CALL_OPERATOR):
        self.expect_delimiter(scanner.METHOD_CALL_OPERATOR)
        name = self.expect_identifier()
        args = self.arguments()
        expr = self.call_prefix(expr, name, args)
      elif self.token().is_delimiter(scanner.DELEGATE_CALL_OPERATOR):
        self.expect_delimiter(scanner.DELEGATE_CALL_OPERATOR)
        fun = self.atomic_expression()
        args = self.arguments()
        expr = ast.Call(expr, fun, args)
      else:
        args = self.arguments()
        expr = ast.Call(ast.This(), expr, args)
    return expr

  def new_expression(self):
    self.expect_keyword(NEW)
    if self.token().is_delimiter('{'):
      recv = Global('Object')
      args = Arguments([], {})
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
    elif self.token().is_keyword(NULL):
      self.expect_keyword(NULL)
      return ast.Literal(ast.Null())
    elif self.token().is_string():
      value = self.expect_string()
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
    else:
      self.unexpected_token()

  def arguments(self):
    self.expect_delimiter('(')
    args = [ ]
    keywords = [ ]
    def parse_argument():
      expr = self.expression(False)
      if self.token().is_delimiter(':') and expr.is_identifier():
        self.expect_delimiter(':')
        keywords[expr.name] = len(args)
        expr = self.expression(False)
      args.append(expr)
    if not self.token().is_delimiter(')'):
      parse_argument()
    while self.token().is_delimiter(','):
      self.advance()
      parse_argument()
    self.expect_delimiter(')')
    return ast.Arguments(args, keywords)

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
    self.scope_ = LocalScope(self.scope_, symbols)

  def pop_scope(self):
    self.scope_ = self.scope().parent()
