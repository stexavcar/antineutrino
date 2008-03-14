# -*- coding: utf-8 -*-

# --- E x c e p t i o n s ---

class CompilationError(Exception):

  def __init__(self, pos, message):
    Exception.__init__(self, message)
    self.message_ = message
    self.pos_ = pos
  
  def position(self):
    return self.pos_
  
  def message(self):
    return self.message_


class SyntaxError(CompilationError):

  def __init__(self, pos, message):
    CompilationError.__init__(self, pos, message)


# --- Keywords ---


def to_map(list):
  """Creates a map from a list (lookup is faster in a map than in a
  list)"""
  result = { }
  for elm in list:
    result[elm] = True
  return result


# The set of keywords
KEYWORDS = to_map([
  'def', 'this', 'is', 'if', 'else', 'while', 'do', 'return', 'null',
  'true', 'false', 'internal', 'operator', 'new', 'in', 'fn', 'and',
  'or', 'not', 'on', 'raise', 'protocol', 'native', 'static', 'task',
  'yield'
])


# The set of delimiters
DELIMITERS = to_map([
  '(', ')', ';', ':', ':=', '{', '}', u'·', '.', ',', '[', ']',
  u'«', u'»', u'‹', u'›'
])


DELEGATE_CALL_OPERATOR = u'·'
METHOD_CALL_OPERATOR = '.'


# The set of characters that can start a delimiter
DELIMITER_STARTS = to_map([ c[0] for c in DELIMITERS.keys() ])


CIRCUMFIXES = {
  '|': '|',
  u'‖': u'‖',
  u'⌊': u'⌋',
  u'⌈': u'⌉'
}


OPERATOR_CHARS = to_map([
  '+', '-', '*', '/', '%', '~',
  '<', '>', '=', '≠', '≥', '≡', '≤', '∈', '∉',
  '⊂', '⊄', '⊃', '⊅', '⊆', '⊈', '⊇', '⊉'
])
OPERATOR_CHARS.update(CIRCUMFIXES)


# The operators that are actually delimiters
RESERVED = to_map([
  '->'
])


# --- Tokens ---

class Position(object):
  
  def __init__(self, doc, start, end):
    self.doc_ = doc
    self.start_ = start
    self.end_ = end
  
  def document(self):
    return self.doc_
  
  def start(self):
    return self.start_
  
  def end(self):
    return self.end_
  
  def __str__(self):
    return "#<a Position: %i-%i>" % (self.start(), self.end())


class Token(object):
  
  def __init__(self, pos, value):
    self.pos_ = pos
    self.value_ = value
  
  def position(self):
    return self.pos_
  
  def value(self):
    return self.value_
  
  def is_documentation(self):
    return False
  
  def is_identifier(self):
    return False
  
  def is_string(self):
    return False
  
  def is_operator(self, value = None):
    return False

  def is_keyword(self, value = None):
    return False
  
  def is_delimiter(self, value):
    return False

  def is_number(self):
    return False


class Keyword(Token):

  def __init__(self, value, pos):
    super(Keyword, self).__init__(pos, value)
  
  def is_keyword(self, value = None):
    return (not value) or (self.value() == value)
  
  def __str__(self):
    return "keyword %s" % self.value()


class Ident(Token):

  def __init__(self, value, pos):
    super(Ident, self).__init__(pos, value)
  
  def is_identifier(self):
    return True


class Number(Token):

  def __init__(self, value, pos):
    super(Number, self).__init__(pos, value)

  def is_number(self):
    return True


class Delimiter(Token):

  def __init__(self, value, pos):
    super(Delimiter, self).__init__(pos, value)
  
  def is_delimiter(self, value):
    return self.value() == value


class Documentation(Token):

  def __init__(self, value, pos):
    super(Documentation, self).__init__(pos, value)
  
  def is_documentation(self):
    return True


class Operator(Token):

  def __init__(self, value, pos):
    super(Operator, self).__init__(pos, value)
  
  def is_operator(self, value = None):
    return (not value) or (self.value() == value)


class String(Token):

  def __init__(self, value, pos):
    super(String, self).__init__(pos, value)
  
  def is_string(self):
    return True


class EOF(Token):
  
  def __init__(self):
    super(EOF, self).__init__(None, None)

# --- Character predicates ---

def is_space(char):
  return char.isspace()


def is_ident_start(char):
  return char.isalpha() or (char == '_')


def is_digit(char):
  return char.isdigit()


def is_ident_part(char):
  return char.isdigit() or is_ident_start(char)


def is_operator(char):
  return char in OPERATOR_CHARS


def is_circumfix_operator(char):
  return char in CIRCUMFIXES


def circumfix_match(char):
  return CIRCUMFIXES[char]


# --- Scanner ---

class Scanner(object):
  
  def __init__(self, reader):
    self.reader = reader
    self.skip_whitespace()
    self.parser_ = None
  
  def set_parser(self, parser):
    self.parser_ = parser
  
  def parser(self):
    return self.parser_
  
  def has_more(self):
    return self.reader.has_more()

  def new_position(self, start, end_delta = -1):
    return Position(self.reader.doc, start, self.reader.position() + end_delta)

  def advance(self):
    if not self.has_more(): return EOF()
    current = self.reader.current()
    if is_ident_start(current):
      result = self.scan_ident()
    elif current in DELIMITER_STARTS:
      result = self.scan_delimiter()
    elif current == '#':
      result = self.scan_documentation()
    elif current == '"':
      result = self.scan_string()
    elif is_operator(current):
      result = self.scan_operator()
    elif is_digit(current):
      result = self.scan_number()
    else:
      pos = self.reader.position()
      raise SyntaxError(self.new_position(pos, 0),
          "Unexpected character '%s'" % current)
    self.skip_whitespace()
    self.current_ = result
    return result
  
  def current(self):
    return self.current_
  
  def scan_string(self):
    start = self.reader.position()
    terms = [ ]
    self.reader.advance()
    term = ''
    while self.reader.has_more() and self.reader.current() != '"':
      if self.reader.current() == '$':
        self.reader.advance()
        current = self.reader.current()
        if current == '{':
          if len(term) > 0:
            terms.append(term)
            term = ''
          self.reader.advance()
          self.advance()
          expr = self.parser().expression(False)
          terms.append(expr)
        else:
          raise SyntaxError(self.new_position(self.reader.position(), 0),
              "Unexpected control character '%s'" % current)
      else:
        term += self.reader.current()
        self.reader.advance()
    if self.has_more(): self.reader.advance()
    if len(term) > 0:
      terms.append(term)
    return String(terms, self.new_position(start))
    
  def scan_documentation(self):
    start = self.reader.position()
    value = ''
    while self.reader.has_more() and self.reader.current() == '#':
      while self.reader.has_more() and self.reader.current() != "\n":
        value += self.reader.current()
        self.reader.advance()
      self.skip_whitespace()
    return Documentation(value, self.new_position(start))

  def scan_number(self):
    start = self.reader.position()
    value = ''
    while self.reader.has_more() and is_digit(self.reader.current()):
      value += self.reader.current()
      self.reader.advance()
      if not is_digit(self.reader.current()):
        self.skip_whitespace()
    return Number(int(value), self.new_position(start))

  def scan_delimiter(self):
    start = self.reader.position()
    value = ''
    is_valid = True
    while self.reader.has_more():
      prev_value = value
      value += self.reader.current()
      if value in DELIMITERS:
        self.reader.advance()
      else:
        return Delimiter(prev_value, self.new_position(start))
  
  def scan_operator(self):
    """Reads the next operator"""
    start = self.reader.position()
    value = ''
    while self.reader.has_more() and is_operator(self.reader.current()):
      value += self.reader.current()
      self.reader.advance()
    if value in RESERVED: return Delimiter(value, self.new_position(start))
    else: return Operator(value, self.new_position(start))
  
  def scan_ident(self):
    """Reads the next identifier or keyword"""
    start = self.reader.position()
    value = ''
    while self.reader.has_more() and is_ident_part(self.reader.current()):
      value += self.reader.current()
      self.reader.advance()
    pos = self.new_position(start)
    if value in KEYWORDS: return Keyword(value, pos)
    else: return Ident(value, pos)

  def skip_whitespace(self):
    """Skip whitespace and comments until the next "hard" token"""
    keep_going = True
    while keep_going:
      # This variable is later set to true if we need to go again
      keep_going = False
      while self.reader.has_more() and is_space(self.reader.current()):
        self.reader.advance()
      # Skip single-line comments
      if self.reader.current() == '/' and self.reader.next() == '/':
        keep_going = True
        while self.reader.has_more() and self.reader.current() != '\n':
          self.reader.advance()
      # Skip multiline comments
      elif self.reader.current() == '(' and self.reader.next() == '*':
        keep_going = True
        def at_end():
          return self.reader.current() == '*' and self.reader.next() == ')'
        while self.reader.has_more() and not at_end():
          self.reader.advance()
        if self.reader.has_more() and self.reader.has_next():
          self.reader.advance()
          self.reader.advance()
