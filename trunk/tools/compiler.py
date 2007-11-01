#!/usr/bin/python

# A simple compiler that produces binaries to be executed by the
# neutrino runtime.  This is not the "real" compiler but is only used
# when bootstrapping the runtime.

from optparse import OptionParser, Option
from os import path

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
  def is_ident(self):
    return True

class Number(Token):
  def __init__(self, value):
    self.value = value
  def is_number(self):
    return True

class String(Token):
  def __init__(self, value):
    self.value = value
  def is_string(self):
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

KEYWORDS = [
  'def', 'this', 'is', 'class', 'if', 'else', 'while', 'do', 'return',
  'null', 'true', 'false'
]

# ---------------------
# --- S c a n n e r ---
# ---------------------

def is_space(char):
  return char.isspace()

def is_ident_start(char):
  return char.isalpha()

def is_digit(char):
  return char.isdigit()

def is_ident_part(char):
  return char.isdigit() or is_ident_start(char)

class Scanner:

  def __init__(self, input):
    self.input = input
    self.cursor = 0

  def current(self):
    if self.has_more(): return self.input[self.cursor]
    else: return None

  def advance(self):
    if self.has_more(): self.cursor += 1
    else: raise "OutOfBounds"

  def has_more(self):
    return self.cursor < len(self.input)

  def tokenize(self):
    self.skip_whitespace()
    tokens = []
    while self.has_more():
      token = self.get_next_token()
      tokens.append(token)
      self.skip_whitespace()
    return tokens

  def scan_ident(self):
    value = ''
    while self.has_more() and is_ident_part(self.current()):
      value += self.current()
      self.advance()
    if value in KEYWORDS: return Keyword(value)
    else: return Ident(value)

  def scan_number(self):
    value = 0
    while self.has_more() and is_digit(self.current()):
      value = 10 * value + (ord(self.current()) - ord('0'))
      self.advance()
    return Number(value)

  def scan_string(self):
    value = ''
    self.advance()
    while self.has_more() and self.current() != '"':
      value += self.current()
      self.advance()
    if self.has_more(): self.advance();
    return String(value)

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
      return Delimiter(':')
    elif c == ',':
      return Delimiter(',')
    elif c == '.':
      return Delimiter('.')
    elif c == '{':
      return Delimiter('{')
    elif c == '}':
      return Delimiter('}')
    elif c == '-':
      if not self.has_more(): return Delimiter('-')
      c = self.current()
      if c == '>':
        self.advance()
        return Delimiter('->')
      else:
        return Delimiter('-')
    else:
      raise c

  def get_next_token(self):
    if is_ident_start(self.current()):
      return self.scan_ident()
    elif is_digit(self.current()):
      return self.scan_number()
    elif self.current() == '"':
      return self.scan_string();
    else:
      return self.scan_delimiter()

  def skip_whitespace(self):
    while self.has_more() and is_space(self.current()):
      self.advance()

# -------------------
# --- P a r s e r ---
# -------------------

BUILTINS = {
  'True':   0,
  'False':  1,
  'String': 2
}

class Parser:

  def __init__(self, tokens):
    self.tokens = tokens
    self.cursor = 0

  # Returns the current token, or <eof> when reaching the end of the
  # input
  def token(self):
    if self.cursor >= len(self.tokens): return EOS()
    else: return self.tokens[self.cursor]

  def parse_error(self):
    raise "SyntaxError: '" + str(self.token()) + "'"

  def expect_keyword(self, expected):
    if not self.token().is_keyword(expected):
      print "Expected " + str(expected) + ", found " + self.token().value
      self.parse_error()
    self.advance()

  def expect_delimiter(self, expected):
    if not self.token().is_delimiter(expected):
      print "Expected " + str(expected) + ", found " + self.token().value
      self.parse_error()
    self.advance()

  # Ensures that the current token is an identifier and then advances
  # the cursor.  If the current token is not correct, an error is
  # reported.
  def expect_ident(self):
    if not self.token().is_ident():
      print "Expected identifier, found " + str(token)
      self.parse_error()
    name = self.token().name
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

  # Advances the cursor one step
  def advance(self):
    self.cursor = self.cursor + 1

  # <program>
  #   -> <declaration>*
  def parse_program(self):
    decls = []
    while not self.token().is_eof():
      decl = self.parse_declaration()
      decls.append(decl)
    return Program(decls)

  # <declaration>
  #   -> <definition>
  #   -> <class_declaration>
  def parse_declaration(self):
    if self.token().is_keyword('def'):
      return self.parse_definition()
    else:
      return self.parse_class_declaration()
  
  # <class_declaration>
  #   -> 'class' $ident '{' <member_declaration>* '}'
  def parse_class_declaration(self):
    self.expect_keyword('class')
    name = self.expect_ident()
    index = BUILTINS[name]
    self.expect_delimiter('{')
    members = []
    while not self.token().is_delimiter('}'):
      member = self.parse_member_declaration()
      members.append(member)
    self.expect_delimiter('}')
    return BuiltinClass(index, members)

  # <member_declaration>
  #   -> 'def' <method_header> <method_body>
  def parse_member_declaration(self):
    self.expect_keyword('def')
    name = self.expect_ident()
    params = self.parse_params('(', ')')
    body = self.parse_function_body()
    return Method(name, params, body)

  # <definition>
  #   -> 'def' $ident ':=' <expression> ';'
  #   -> 'def' $ident '(' <params> ')' <function-body>
  def parse_definition(self):
    self.expect_keyword('def')
    name = self.expect_ident()
    if self.token() == ':=':
      self.expect(':=')
      value = self.parse_expression(False)
      self.expect_delimiter(';')
      return Definition(name, value)
    else:
      params = self.parse_params('(', ')')
      body = self.parse_function_body()
      return Definition(name, Lambda(params, body))

  # <function-body>
  #   -> '->' <expression>
  #   -> <block-expression>
  def parse_function_body(self):
    if self.token().is_delimiter('->'):
      self.expect_delimiter('->')
      value = self.parse_expression(False)
      self.expect_delimiter(';')
      return Return(value)
    elif self.token().is_delimiter('{'):
      exprs = []
      self.expect_delimiter('{')
      while not self.token().is_delimiter('}'):
        expr = self.parse_expression(True)
        exprs.append(expr)
      self.expect_delimiter('}')
      exprs.append(Return(Void()))
      return Sequence(exprs)

  # <params>
  #   -> $ident *: ','
  def parse_params(self, start, end):
    params = []
    self.expect_delimiter(start)
    if self.token().is_ident():
      params.append(self.expect_ident())
      while self.token().is_delimiter(','):
        self.expect_delimiter(',')
        params.append(self.expect_ident())
    self.expect_delimiter(end)
    return params

  # <expression>
  #   -> <control_expression>
  def parse_expression(self, is_toplevel):
    return self.parse_control_expression(is_toplevel)

  # <arguments>
  #   -> <expression> *: ','
  def parse_arguments(self, start, end):
    args = []
    self.expect_delimiter(start)
    if not self.token().is_delimiter(end):
      expr = self.parse_expression(False)
      args.append(expr)
    while self.token().is_delimiter(','):
      self.advance()
      expr = self.parse_expression(False)
      args.append(expr)
    self.expect_delimiter(end)
    return args

  # <control_expression>
  #   -> 'return' <expression>
  #   -> <conditional-expression>
  def parse_control_expression(self, is_toplevel):
    if self.token().is_keyword('return'):
      self.expect_keyword('return')
      value = self.parse_expression(False)
      if is_toplevel: self.expect_delimiter(';')
      return Return(value)
    elif self.token().is_keyword('if'):
      return self.parse_conditional_expression(is_toplevel)
    else:
      return self.parse_call_expression()

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

  # <call_expression>
  #   -> <atomic_expression> <arguments>
  #   -> <atomic_expression> ':' <atomic expression> <arguments>
  #   -> <atomic_expression> '.' $name <arguments>
  def parse_call_expression(self):
    expr = self.parse_atomic_expression()
    if self.token().is_delimiter('.'):
      self.expect_delimiter('.')
      name = self.expect_ident()
      args = self.parse_arguments('(', ')')
      return Invoke(expr, name, args)
    else:
      if self.token().is_delimiter(':'):
        self.expect_delimiter(':')
        recv = expr
        expr = self.parse_atomic_expression()
      elif self.token().is_delimiter('('):
        recv = This()
      if self.token().is_delimiter('('):
        args = self.parse_arguments('(', ')')
        return Call(recv, expr, args)
      else:
        return expr

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
      return Literal(value)
    elif self.token().is_string():
      value = self.token().value
      self.advance()
      return Literal(value)
    elif self.token().is_ident():
      name = self.token().name
      self.advance()
      return Identifier(name)
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
    elif self.token().is_delimiter('('):
      self.expect_delimiter('(')
      value = self.parse_expression(False)
      self.expect_delimiter(')')
      return value
    else:
      self.parse_error()

# Validates the parsed command-line arguments
def process_args(values, files):
  if not values.out:
    print "An output file must be specified."
    return False
  output_file = values.out
  for file in files:
    if not path.exists(file):
      print "Could not find file " + file + "."
      return False
  return True

# -------------------------------
# --- S y n t a x   T r e e s ---
# -------------------------------

class SyntaxTree:
  pass

class Program(SyntaxTree):
  def __init__(self, decls):
    self.decls = decls
  def to_sexp(self):
    result = '(program '
    first = True
    for decl in self.decls:
      if first: first = False
      else: result += ' '
      result += decl.to_sexp()
    result += ')'
    return result

class Definition(SyntaxTree):
  def __init__(self, name, value):
    self.name = name
    self.value = value
  def to_sexp(self):
    return '(define "' + self.name + '" ' + self.value.to_sexp() + ')'

class BuiltinClass(SyntaxTree):
  def __init__(self, index, members):
    self.index = index
    self.members = members
  def to_sexp(self):
    result = '(builtin ' + str(self.index) + ' ('
    first = True
    for member in self.members:
      if first: first = False
      else: result += ' '
      result += member.to_sexp()
    return result + '))'

class Method(SyntaxTree):
  def __init__(self, name, params, body):
    self.name = name
    self.params = params
    self.body = body
  def to_sexp(self):
    body = lambda_to_sexp(self.params, self.body)
    return '(method "' + self.name + '" ' + body + ')'

class Expression(SyntaxTree):
  def compile(self, state):
    self.emit(state)
    literals = state.literals
    result = '('
    first = True
    for instr in state.code:
      if first: first = False
      else: result += ' '
      result += str(instr)
    result += ') ('
    first = True
    for literal in state.literals:
      if first: first = False
      else: result += ' '
      result += serialize_value(literal)
    result += ')'
    return result

class Literal(Expression):
  def __init__(self, value):
    self.value = value
  def emit(self, state):
    index = state.literal_index(self.value)
    state.write(LITERAL, index)
  def to_sexp(self):
    return '(number ' + str(self.value) + ')'

def lambda_to_sexp(params, body):
  state = CodeGeneratorState()
  state.scopes.append(params)
  code = body.compile(state)
  return '(lambda ' + str(len(params)) + ' ' + code + ')'

class Lambda(Expression):
  def __init__(self, params, body):
    self.params = params
    self.body = body
  def to_sexp(self):
    return lambda_to_sexp(self.params, self.body)

class Identifier(Expression):
  def __init__(self, name):
    self.name = name
  def emit(self, state):
    top_scope = []
    if len(state.scopes) > 0:
      top_scope = state.scopes[0]
    try:
      scope_index = len(top_scope) - top_scope.index(self.name) - 1
      state.write(ARGUMENT, scope_index)
    except ValueError:
      index = state.literal_index(self.name)
      state.write(GLOBAL, index)

class Call(Expression):
  def __init__(self, recv, fun, args):
    self.recv = recv
    self.fun = fun
    self.args = args
  def emit(self, state):
    self.recv.emit(state)
    self.fun.emit(state)
    for arg in self.args:
      arg.emit(state)
    state.write(CALL, len(self.args))
    state.write(SLAP, len(self.args) + 1)

class Invoke(Expression):
  def __init__(self, recv, name, args):
    self.recv = recv
    self.name = name
    self.args = args
  def emit(self, state):
    self.recv.emit(state)
    state.write(VOID)
    for arg in self.args:
      arg.emit(state)
    name_index = state.literal_index(self.name)
    state.write(INVOKE, name_index, len(self.args))
    state.write(SLAP, len(self.args) + 1)

class This(Expression):
  def emit(self, state):
    assert len(state.scopes) > 0
    top_scope = state.scopes[0]
    state.write(ARGUMENT, len(top_scope) + 1)

class Void(Expression):
  def emit(self, state):
    state.write(VOID)

class Null(Expression):
  def emit(self, state):
    state.write(NULL)

class Thrue(Expression):
  def emit(self, state):
    state.write(TRUE)

class Fahlse(Expression):
  def emit(self, state):
    state.write(FALSE)

class Return(Expression):
  def __init__(self, value):
    self.value = value
  def emit(self, state):
    self.value.emit(state)
    state.write(RETURN)

class Sequence(Expression):
  def __init__(self, exprs):
    self.exprs = exprs
  def emit(self, state):
    if len(self.exprs) == 0:
      state.write(VOID)
    else:
      first = True
      for expr in self.exprs:
        if first: first = False
        else: state.write(POP, 1)
        expr.emit(state)

class Conditional(Expression):
  def __init__(self, cond, then_part, else_part):
    self.cond = cond
    self.then_part = then_part
    self.else_part = else_part
  def emit(self, state):
    self.cond.emit(state)
    if_true_jump = state.write(IF_TRUE, PLACEHOLDER)
    self.else_part.emit(state)
    end_jump = state.write(GOTO, PLACEHOLDER)
    state.bind(if_true_jump)
    self.then_part.emit(state)
    state.bind(end_jump)

# -----------------------
# --- C o m p i l e r ---
# -----------------------

LITERAL     = 0
RETURN      = 1
GLOBAL      = 2
CALL        = 3
SLAP        = 4
ARGUMENT    = 5
VOID        = 6
NULL        = 7
TRUE        = 8
FALSE       = 9
POP         = 10
IF_TRUE     = 11
GOTO        = 12
INVOKE      = 13
PLACEHOLDER = 0xBADDEAD

class CodeGeneratorState:
  def __init__(self):
    self.code = []
    self.literals = []
    self.scopes = []
  def literal_index(self, value):
    result = len(self.literals)
    self.literals.append(value)
    return result
  def write(self, *args):
    self.code += args
    return len(self.code) - 1
  def bind(self, offset):
    assert self.code[offset] == PLACEHOLDER
    self.code[offset] = len(self.code)

def serialize_value(value):
  if type(value) is str:
    return '"' + value + '"'
  else:
    return str(value)

def compile(str):
  scanner = Scanner(str)
  tokens = scanner.tokenize()
  parser = Parser(tokens)
  return parser.parse_program()

def emit(tree, out):
  str = tree.to_sexp()
  out.write(str)
  out.write('\n')

# ---------------
# --- M a i n ---
# ---------------

# The set of command-line options
options = [
  Option("-o", "--output", dest='out', help='output file', nargs=1)
]

def main():
  parser = OptionParser(option_list=options)
  (values, files) = parser.parse_args()
  if not process_args(values, files):
    parser.print_help()
    return
  output = open(values.out, 'w')
  for file in files:
    source = open(file).read()
    tree = compile(source)
    emit(tree, output)
  output.close()


if __name__ == '__main__':
  main()
