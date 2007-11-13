#!/usr/bin/python
#coding=utf8

# A compiler that produces binaries to be executed by the neutrino
# runtime.  This is not the "real" compiler but is only used
# when bootstrapping the runtime.

from optparse import OptionParser, Option
from os import path
import re, string, struct

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
  def is_ident(self):
    return True

class Operator(Token):
  def __init__(self, name):
    self.name = name
  def is_operator(self):
    return True

class NumberToken(Token):
  def __init__(self, value):
    self.value = value
  def is_number(self):
    return True

class StringToken(Token):
  def __init__(self, value):
    self.value = value
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
  return char.isalpha() or (char == '_')

def is_digit(char):
  return char.isdigit()

def is_ident_part(char):
  return char.isdigit() or is_ident_start(char)

def is_operator(char):
  return char == '+' or char == '-' or char == '>' or char == '*' \
      or char == '/' or char == '%' or char == '='

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
    return NumberToken(value)

  def scan_string(self):
    value = ''
    self.advance()
    while self.has_more() and self.current() != '"':
      value += self.current()
      self.advance()
    if self.has_more(): self.advance()
    return StringToken(value)
  
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
      return Delimiter(':')
    elif c == ',':
      return Delimiter(',')
    elif c == '.':
      return Delimiter('.')
    elif c == '{':
      return Delimiter('{')
    elif c == '}':
      return Delimiter('}')
    elif c == '•':
      return Delimiter('•')
    elif c == '-':
      if not self.has_more(): return Delimiter('-')
      c = self.current()
      if c == '>':
        self.advance()
        return Delimiter('->')
      else:
        return Delimiter('-')
    else:
      raise "Unexpected character " + c
  
  def get_next_token(self):
    if is_ident_start(self.current()):
      return self.scan_ident()
    elif is_digit(self.current()):
      return self.scan_number()
    elif self.current() == '"':
      return self.scan_string();
    elif is_operator(self.current()):
      return self.scan_operator()
    elif self.current() == '#':
      return self.scan_documentation()
    else:
      return self.scan_delimiter()

  def skip_whitespace(self):
    while self.has_more() and is_space(self.current()):
      self.advance()


# -------------------
# --- P a r s e r ---
# -------------------

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
    else:
      return self.parse_class_declaration(modifiers)
  
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
  #   -> 'class' $ident (':' $super)? '{' <member_declaration>* '}'
  def parse_class_declaration(self, modifiers):
    self.expect_keyword('class')
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
    if 'internal' in modifiers:
      info = BUILTIN_CLASSES[name]
      return BuiltinClass(info, members, parent)
    else:
      return Class(name, members, parent)

  # <member_declaration>
  #   -> <modifiers> 'def' <method_header> <method_body>
  def parse_member_declaration(self, class_name):
    doc = self.parse_doc_opt()
    modifiers = self.parse_modifiers()
    self.expect_keyword('def')
    name = self.parse_member_name()
    params = self.parse_params('(', ')')
    if 'internal' in modifiers:
      self.expect_delimiter(';')
      key = (class_name, name)
      if not key in BUILTIN_METHODS:
        raise "Unknown built-in method " + class_name + " " + name
      index = BUILTIN_METHODS[key]
      body = InternalCall(index, len(params))
    else:
      body = self.parse_function_body()
    return Method(name, params, body)

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
      if 'internal' in modifiers:
        index = BUILTIN_FUNCTIONS[name]
        body = InternalCall(index, len(params));
        self.expect_delimiter(';')
      else:
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
  #   -> <operator-expression> ';'
  def parse_control_expression(self, is_toplevel):
    if self.token().is_keyword('return'):
      self.expect_keyword('return')
      value = self.parse_expression(False)
      if is_toplevel: self.expect_delimiter(';')
      return Return(value)
    elif self.token().is_keyword('if'):
      return self.parse_conditional_expression(is_toplevel)
    else:
      value = self.parse_operator_expression()
      if (is_toplevel): self.expect_delimiter(';')
      return value
  
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
      if self.token().is_delimiter('•'):
        self.expect_delimiter('•')
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
      return Literal(HEAP.new_number(value))
    elif self.token().is_string():
      value = self.token().value
      self.advance()
      return Literal(HEAP.new_string(value))
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
  def load(self, namespace):
    for decl in self.decls:
      decl.define(namespace)
  def resolve(self, namespace):
    for decl in self.decls:
      decl.resolve(namespace)

class Definition(SyntaxTree):
  def __init__(self, name, value):
    self.name = name
    self.value = value
  def define(self, namespace):
    value = self.value.compile()
    HEAP.toplevel[HEAP.new_string(self.name)] = value
    namespace[self.name] = self
  def resolve(self, namespace):
    pass

class BuiltinClass(SyntaxTree):
  def __init__(self, info, members, parent):
    self.info = info
    self.members = members
    self.parent = parent
  def define(self, namespace):
    instance_type_name = self.info.instance_type
    instance_type_index = globals()[instance_type_name + '_TYPE']
    methods = HEAP.new_tuple(values = map(Method.compile, self.members))
    chlass = HEAP.new_class(instance_type_index)
    chlass.set_methods(methods)
    HEAP.set_root(self.info.root_index, chlass)
    self.image = chlass
  def resolve(self, namespace):
    if self.parent:
      parent_value = namespace[self.parent]
      self.image.set_parent(parent_value.image)

class Class(SyntaxTree):
  def __init__(self, name, members, parent):
    self.name = name
    self.members = members
    self.parent = parent
  def define(self, namespace):
    methods = HEAP.new_tuple(values = map(Method.compile, self.members))
    chlass = HEAP.new_class(OBJECT_TYPE)
    chlass.set_methods(methods)
    HEAP.toplevel[HEAP.new_string(self.name)] = chlass
    self.image = chlass
    namespace[self.name] = self
  def resolve(self, namespace):
    if self.parent:
      parent_value = namespace[self.parent]
      self.image.set_parent(parent_value.image)

class Method(SyntaxTree):
  def __init__(self, name, params, body):
    self.name = name
    self.params = params
    self.body = body
  def compile(self):
    body = compile_lambda(self.params, self.body)
    return HEAP.new_method(HEAP.new_string(self.name), body)

class Expression(SyntaxTree):
  pass

class Literal(Expression):
  def __init__(self, value):
    self.value = value
  def emit(self, state):
    index = state.literal_index(self.value)
    state.write(OC_PUSH, index)

def compile_lambda(params, body):
  state = CodeGeneratorState()
  state.scopes.append(params)
  body.emit(state)
  code = HEAP.new_code(state.code)
  literals = HEAP.new_tuple(values = state.literals)
  return HEAP.new_lambda(len(params), code, literals)

class Lambda(Expression):
  def __init__(self, params, body):
    self.params = params
    self.body = body
  def compile(self):
    return compile_lambda(self.params, self.body)

class Identifier(Expression):
  def __init__(self, name):
    self.name = name
  def emit(self, state):
    top_scope = []
    if len(state.scopes) > 0:
      top_scope = state.scopes[0]
    try:
      scope_index = len(top_scope) - top_scope.index(self.name) - 1
      state.write(OC_ARGUMENT, scope_index)
    except ValueError:
      index = state.literal_index(HEAP.new_string(self.name))
      state.write(OC_GLOBAL, index)

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
    state.write(OC_CALL, len(self.args))
    state.write(OC_SLAP, len(self.args) + 1)

class Invoke(Expression):
  def __init__(self, recv, name, args):
    self.recv = recv
    self.name = name
    self.args = args
  def emit(self, state):
    self.recv.emit(state)
    state.write(OC_VOID)
    for arg in self.args:
      arg.emit(state)
    name_index = state.literal_index(HEAP.new_string(self.name))
    state.write(OC_INVOKE, name_index, len(self.args))
    state.write(OC_SLAP, len(self.args) + 1)

class InternalCall(Expression):
  def __init__(self, index, argc):
    self.index = index
    self.argc = argc
  def emit(self, state):
    state.write(OC_INTERNAL, self.index, self.argc)
    state.write(OC_RETURN)

class This(Expression):
  def emit(self, state):
    assert len(state.scopes) > 0
    top_scope = state.scopes[0]
    state.write(OC_ARGUMENT, len(top_scope) + 1)

class Void(Expression):
  def emit(self, state):
    state.write(OC_VOID)

class Null(Expression):
  def emit(self, state):
    state.write(OC_NULL)

class Thrue(Expression):
  def emit(self, state):
    state.write(OC_TRUE)

class Fahlse(Expression):
  def emit(self, state):
    state.write(OC_FALSE)

class Return(Expression):
  def __init__(self, value):
    self.value = value
  def emit(self, state):
    self.value.emit(state)
    state.write(OC_RETURN)

class Sequence(Expression):
  def __init__(self, exprs):
    self.exprs = exprs
  def emit(self, state):
    if len(self.exprs) == 0:
      state.write(OC_VOID)
    else:
      first = True
      for expr in self.exprs:
        if first: first = False
        else: state.write(OC_POP, 1)
        expr.emit(state)

class Conditional(Expression):
  def __init__(self, cond, then_part, else_part):
    self.cond = cond
    self.then_part = then_part
    self.else_part = else_part
  def emit(self, state):
    self.cond.emit(state)
    if_true_jump = state.write(OC_IF_TRUE, PLACEHOLDER)
    self.else_part.emit(state)
    end_jump = state.write(OC_GOTO, PLACEHOLDER)
    state.bind(if_true_jump)
    self.then_part.emit(state)
    state.bind(end_jump)

# ---------------------------
# --- H e a p   I m a g e ---
# ---------------------------

def tag_as_smi(value):
  return value << 1

def tag_as_object(value):
  assert value & 0x3 is 0
  return value | 0x1

POINTER_SIZE = 4

class Heap:
  kRootCount  = 19
  def __init__(self):
    self.capacity = 1024
    self.cursor = 0
    self.memory = self.capacity * [ tag_as_smi(0) ]
    self.dicts = [ ]
  def initialize(self):
    self.roots = self.new_tuple(Heap.kRootCount)
    self.toplevel = self.new_dictionary()
    self.set_root(TOPLEVEL_ROOT, self.toplevel)
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
  def allocate(self, size):
    if self.cursor + size > self.capacity:
      new_capacity = self.capacity * 2
      new_memory = new_capacity * [ tag_as_smi(0) ]
      for i in xrange(0, self.cursor):
        new_memory[i] = self.memory[i]
      self.capacity = new_capacity
      self.memory = new_memory
    addr = self.cursor * POINTER_SIZE
    self.cursor += size
    return addr
  def new_class(self, instance_type):
    result = ImageClass(self.allocate(ImageClass_Size), instance_type)
    result.set_class(CLASS_TYPE)
    return result
  def new_lambda(self, argc, code, literals):
    result = ImageLambda(self.allocate(ImageLambda_Size), argc, code, literals)
    result.set_class(LAMBDA_TYPE)
    return result
  def new_method(self, name, body):
    result = ImageMethod(self.allocate(ImageMethod_Size), name, body)
    result.set_class(METHOD_TYPE)
    return result
  def new_number(self, value):
    return value
  def new_dictionary(self):
    result = ImageDictionary(self.allocate(ImageDictionary_Size))
    result.set_class(DICTIONARY_TYPE)
    self.dicts.append(result)
    return result
  def new_tuple(self, length = None, values = None):
    if length is None: length = len(values)
    result = ImageTuple(self.allocate(ImageTuple_HeaderSize + length), length)
    result.set_class(TUPLE_TYPE)
    if not values is None:
      for i in xrange(length):
        result[i] = values[i]
    return result
  def new_string(self, contents):
    length = len(contents)
    result = ImageString(self.allocate(ImageString_HeaderSize + length), length)
    result.set_class(STRING_TYPE)
    for i in xrange(length):
      result[i] = ord(contents[i])
    return result
  def new_code(self, contents):
    length = len(contents)
    result = ImageCode(self.allocate(ImageCode_HeaderSize + length), length)
    result.set_class(CODE_TYPE)
    for i in xrange(length):
      result[i] = contents[i]
    return result
  def set_raw_field(self, obj, offset, value):
    assert type(value) is int
    self.set_raw((obj.addr / POINTER_SIZE) + offset, value)
  def set_field(self, obj, offset, value):
    self[(obj.addr / POINTER_SIZE) + offset] = value
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
    write(tag_as_object(self.roots.addr))
    for value in buffer:
      write(value)
    output.close()

class ImageValue:
  pass

class ImageObject(ImageValue):
  def __init__(self, addr):
    self.addr = addr
  def set_class(self, value):
    HEAP.set_field(self, ImageObject_TypeOffset, value)

class ImageClass(ImageObject):
  def __init__(self, addr, instance_type):
    ImageObject.__init__(self, addr)
    self.set_instance_type(instance_type)
  def set_instance_type(self, value):
    HEAP.set_raw_field(self, ImageClass_InstanceTypeOffset, value)
  def set_methods(self, value):
    HEAP.set_field(self, ImageClass_MethodsOffset, value)
  def set_parent(self, value):
    HEAP.set_field(self, ImageClass_SuperOffset, value)

class ImageTuple(ImageObject):
  def __init__(self, addr, length):
    ImageObject.__init__(self, addr)
    self.set_length(length)
  def set_length(self, value):
    HEAP.set_raw_field(self, ImageTuple_LengthOffset, value)
  def __setitem__(self, index, value):
    HEAP.set_field(self, ImageTuple_HeaderSize + index, value)

class HeapNumber(ImageObject):
  def __init__(self, addr, value):
    ImageObject.__init__(self, addr)
    self.set_value(value)
  def set_value(self, value):
    HEAP.set_raw_field(self, Number.kValueOffset, value)

class ImageLambda(ImageObject):
  def __init__(self, addr, argc, code, literals):
    ImageObject.__init__(self, addr)
    self.set_argc(argc)
    self.set_code(code)
    self.set_literals(literals)
  def set_argc(self, value):
    HEAP.set_raw_field(self, ImageLambda_ArgcOffset, value)
  def set_code(self, value):
    HEAP.set_field(self, ImageLambda_CodeOffset, value)
  def set_literals(self, value):
    HEAP.set_field(self, ImageLambda_LiteralsOffset, value)

class ImageMethod(ImageObject):
  def __init__(self, addr, name, body):
    ImageObject.__init__(self, addr)
    self.set_name(name)
    self.set_body(body)
  def set_name(self, value):
    HEAP.set_field(self, ImageMethod_NameOffset, value)
  def set_body(self, value):
    HEAP.set_field(self, ImageMethod_LambdaOffset, value)

class ImageString(ImageObject):
  def __init__(self, addr, length):
    ImageObject.__init__(self, addr)
    self.set_length(length)
  def set_length(self, value):
    HEAP.set_raw_field(self, ImageString_LengthOffset, value)
  def __setitem__(self, index, value):
    HEAP.set_raw_field(self, ImageString_HeaderSize + index, value)

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

def define_opcode(n, NAME, argc):
  globals()['OC_' + NAME] = int(n)

class BuiltinClassInfo:
  def __init__(self, root_index, name, instance_type, class_name):
    self.root_index = root_index
    self.name = name
    self.instance_type = instance_type
    self.class_name = class_name

def define_root(n, Class, name, NAME, allocator):
  globals()[NAME + '_ROOT'] = int(n)

def define_image_object_const(n, Type, Name):
  name = 'Image' + Type + '_' + Name
  globals()[name] = int(n)
  
BUILTIN_CLASSES = { }
CLASSES_BY_ROOT_NAME = { }
def define_builtin_class(Class, name, NAME):
  root_name = NAME + '_CLASS_ROOT'
  if root_name in globals(): root_index = globals()[root_name]
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
  consts.apply('FOR_EACH_OPCODE', define_opcode)
  consts.apply('FOR_EACH_ROOT', define_root)
  consts.apply('FOR_EACH_BUILTIN_CLASS', define_builtin_class)
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

def load_files(files):
  trees = []
  namespace = {}
  for file in files:
    if path.isdir(file):
      files += find_source_files(file)
    else:
      source = open(file).read()
      tree = compile(source)
      tree.load(namespace)
      trees.append(tree)
  for tree in trees:
    tree.resolve(namespace)

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
