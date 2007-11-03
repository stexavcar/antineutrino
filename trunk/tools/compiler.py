#!/usr/bin/python

# A simple compiler that produces binaries to be executed by the
# neutrino runtime.  This is not the "real" compiler but is only used
# when bootstrapping the runtime.

from optparse import OptionParser, Option
from os import path
import re, string

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
  def is_operator(self): return False

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

class Operator(Token):
  def __init__(self, name):
    self.name = name
  def is_operator(self):
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
  'null', 'true', 'false', 'internal', 'operator'
]

RESERVED = [
  '->'
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

def is_operator(char):
  return char == '+' or char == '-' or char == '>'

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
    elif is_operator(self.current()):
      return self.scan_operator()
    else:
      return self.scan_delimiter()

  def skip_whitespace(self):
    while self.has_more() and is_space(self.current()):
      self.advance()

# -------------------
# --- P a r s e r ---
# -------------------

BUILTIN_CLASSES = {
  'True':         0,
  'False':        1,
  'String':       2,
  'SmallInteger': 3
}

BUILTIN_METHODS = {
  ('String',       'length'):  0,
  ('SmallInteger', '+'):       1,
  ('SmallInteger', '-'):       2
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

  def expect_operator(self):
    if not self.token().is_operator():
      print "Expected operator, found " + str(self.token())
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
  
  def is_modifier(self, token):
    return token.is_keyword('internal')

  # <modifiers>
  #   -> <modifier>*
  def parse_modifiers(self):
    modifiers = { }
    while self.is_modifier(self.token()):
      modifiers[self.token().value] = True
      self.advance()
    return modifiers
  
  # <member_name>
  #   -> $ident
  #   -> 'operator' $operator
  def parse_member_name(self):
    if self.token().is_ident():
      return self.expect_ident()
    else:
      self.expect_keyword('operator')
      return self.expect_operator()
  
  # <class_declaration>
  #   -> <modifiers> 'class' $ident '{' <member_declaration>* '}'
  def parse_class_declaration(self):
    modifiers = self.parse_modifiers()
    self.expect_keyword('class')
    name = self.expect_ident()
    index = BUILTIN_CLASSES[name]
    self.expect_delimiter('{')
    members = []
    while not self.token().is_delimiter('}'):
      member = self.parse_member_declaration(name)
      members.append(member)
    self.expect_delimiter('}')
    return BuiltinClass(index, members)

  # <member_declaration>
  #   -> <modifiers> 'def' <method_header> <method_body>
  def parse_member_declaration(self, class_name):
    modifiers = self.parse_modifiers()
    self.expect_keyword('def')
    name = self.parse_member_name()
    params = self.parse_params('(', ')')
    if 'internal' in modifiers:
      self.expect_delimiter(';')
      index = BUILTIN_METHODS[(class_name, name)]
      body = InternalCall(index, len(params))
    else:
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
      return self.parse_operator_expression()
  
  def parse_operator_expression(self):
    expr = self.parse_call_expression()
    while self.token().is_operator():
      op = self.expect_operator()
      right = self.parse_call_expression()
      expr = Invoke(expr, op, [right])
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

# -------------------------------
# --- S y n t a x   T r e e s ---
# -------------------------------

class SyntaxTree:
  pass

class Program(SyntaxTree):
  def __init__(self, decls):
    self.decls = decls
  def load(self):
    for decl in self.decls:
      decl.define()
      return

class Definition(SyntaxTree):
  def __init__(self, name, value):
    self.name = name
    self.value = value
  def define(self):
    Runtime.current().roots().toplevel().set(self.name, self.value.create())

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
    state.write(PUSH, index)
  def to_sexp(self):
    return '(number ' + str(self.value) + ')'

def create_lambda(params, body):
  state = CodeGeneratorState()
  body.emit(state)
  code = HeapCode(state.code)
  literals = HeapTuple(state.literals)
  return HeapLambda(len(params), code, literals)

class Lambda(Expression):
  def __init__(self, params, body):
    self.params = params
    self.body = body
  def create(self):
    return create_lambda(self.params, self.body)

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

class InternalCall(Expression):
  def __init__(self, index, argc):
    self.index = index
    self.argc = argc
  def emit(self, state):
    state.write(INTERNAL, self.index, self.argc)
    state.write(RETURN)
    
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

# ---------------------------
# --- H e a p   I m a g e ---
# ---------------------------

class Roots:
  def __init__(self):
    self.entries = { }
  def initialize(self):
    class_class = HeapClass(CLASS_TYPE, None)
    class_class.chlass = class_class
    self.entries['class_class'] = class_class
    self.entries['code_class'] = HeapClass(CODE_TYPE, class_class)
    self.entries['tuple_class'] = HeapClass(TUPLE_TYPE, class_class)
    self.entries['lambda_class'] = HeapClass(LAMBDA_TYPE, class_class)
    self.entries['string_class'] = HeapClass(STRING_TYPE, class_class)
    self.entries['dictionary_class'] = HeapClass(DICTIONARY_TYPE, class_class)
    self.entries['toplevel'] = HeapDictionary()
  def toplevel(self):
    return self.entries['toplevel']
  def get_class(self, name):
    return self.entries[name + '_class']
  def write_to(self, stream):
    for key, value in self.entries.items():
      index = globals()['SET_' + key.upper()]
      stream.write(index)
      stream.write(value)

CURRENT_RUNTIME = None

class Runtime:
  current_runtime = None
  def __init__(self):
    self.roots_ = Roots()
  def initialize(self):
    self.roots().initialize()
  def roots(self):
    return self.roots_
  def write_to(self, stream):
    self.roots().write_to(stream)
  def set_current(cls, runtime):
    cls.current_runtime = runtime
  set_current = classmethod(set_current)
  def current(cls):
    return cls.current_runtime
  current = classmethod(current)

def get_class(name):
  roots = Runtime.current().roots()
  return roots.get_class(name)

class HeapValue:
  def __init__(self):
    self.offset = None

class HeapObject(HeapValue):
  def __init__(self, chlass):
    HeapValue.__init__(self)
    self.chlass = chlass
  def write_to(self, stream):
    stream.write(self.chlass)

class HeapString(HeapObject):
  def __init__(self, value):
    HeapObject.__init__(self, get_class('string'))
    self.value = value
  def write_to(self, stream):
    stream.write(NEW_STRING)
    stream.write(self.value)

class HeapClass(HeapObject):
  def __init__(self, instance_type, chlass):
    HeapObject.__init__(self, chlass)
    self.instance_type = instance_type
  def write_to(self, stream):
    stream.write(NEW_CLASS)
    HeapObject.write_to(self, stream)
    stream.write(self.instance_type)

class HeapTuple(HeapObject):
  def __init__(self, values):
    HeapObject.__init__(self, get_class('tuple'))
    self.values = values
  def write_to(self, stream):
    stream.write(NEW_TUPLE)
    stream.write(len(self.values))
    for value in self.values:
      stream.write(value)

class HeapLambda(HeapObject):
  def __init__(self, argc, code, literals):
    HeapObject.__init__(self, get_class('lambda'))
    self.argc = argc
    self.code = code
    self.literals = literals
  def write_to(self, stream):
    stream.write(NEW_LAMBDA)
    stream.write(self.argc)
    stream.write(self.code)
    stream.write(self.literals)

class HeapDictionary(HeapObject):
  def __init__(self):
    HeapObject.__init__(self, get_class('dictionary'))
    self.contents = { }
  def set(self, key, value):
    self.contents[HeapString(key)] = value
  def write_to(self, stream):
    stream.write(NEW_DICTIONARY)
    stream.write(len(self.contents))
    for key, value in self.contents.items():
      stream.write(key)
      stream.write(value)

class HeapCode(HeapObject):
  def __init__(self, buffer):
    HeapObject.__init__(self, get_class('code'))
    self.buffer = buffer
  def write_to(self, stream):
    stream.write(NEW_CODE)
    stream.write(len(self.buffer))
    for instr in self.buffer:
      stream.write(instr)

class ImageOutputStream:
  def __init__(self):
    self.buffer = []
    self.registers = {}
  def write(self, value):
    self.buffer.append(value)
  def cursor(self):
    return len(self.buffer)
  def ensure_register(self, offset):
    if offset in self.registers:
      return self.registers[offset]
    next = len(self.registers)
    self.registers[offset] = next
    return next

class ImageStream:
  def __init__(self):
    self.data = []
  def write(self, value):
    self.data.append(value)
  def flush(self, buffer):
    offset = 0
    while offset < len(self.data):
      elem = self.data[offset]
      if type(elem) is int:
        buffer.write(elem)
      elif type(elem) is str:
        buffer.write(len(elem))
        for char in elem:
          buffer.write(ord(char))
      else:
        if elem.offset:
          register = buffer.ensure_register(elem.offset)
          buffer.write(LOAD_REGISTER)
          buffer.write(register)
        else:
          elem.offset = buffer.cursor()
          sub_stream = ImageStream()
          elem.write_to(sub_stream)
          sub_stream.flush(buffer)
      offset = offset + 1
    return buffer

# -----------------------
# --- C o m p i l e r ---
# -----------------------

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

def define_type_tag(n, TYPE, Type):
  globals()[TYPE + '_TYPE'] = int(n)

def define_opcode(NAME, argc, n):
  globals()[NAME] = int(n)

def define_instruction(n, NAME):
  globals()[NAME] = int(n)

def main():
  parser = OptionParser(option_list=options)
  (values, files) = parser.parse_args()
  if not process_args(values, files):
    parser.print_help()
    return
  consts = read_consts(values.consts)
  consts.apply('FOR_EACH_DECLARED_TYPE', define_type_tag)
  consts.apply('FOR_EACH_OPCODE', define_opcode)
  consts.apply('FOR_EACH_INSTRUCTION', define_instruction)
  Runtime.set_current(Runtime())
  Runtime.current().initialize()
  for file in files:
    source = open(file).read()
    tree = compile(source)
    tree.load()
  stream = ImageStream()
  Runtime.current().write_to(stream)
  data = ImageOutputStream()
  stream.flush(data)
  print data.buffer
  print data.registers
  output = open(values.out, 'w')
  output.close()


if __name__ == '__main__':
  main()
