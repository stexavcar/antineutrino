from os import listdir
from os.path import join, abspath
import re


class Matcher(object):
  
  def __init__(self, key):
    self.key = key
  
  def get_key(self):
    return self.key


class Token(Matcher):
  
  def __init__(self, value):
    super(Token, self).__init__(None)
    self.value = value
  
  def match(self, scan):
    while scan.has_more() and scan.current().isspace():
      scan.advance()
    for i in xrange(len(self.value)):
      if scan.current() == self.value[i]:
        scan.advance()
      else:
        return (False, None)
    return (True, None)


class Expression(Matcher):
  
  def __init__(self, key):
    super(Expression, self).__init__(key)
  
  def match(self, scan):
    # Skip spaces
    while scan.has_more() and scan.current().isspace():
      scan.advance()
    if not scan.has_more():
      return (False, None)
    # Skip identifier / number
    if scan.current().isalnum():
      while scan.has_more() and scan.current().isalnum():
        scan.advance()
      return (True, scan.position())
    if scan.current() == "'":
      scan.advance()
      while scan.has_more() and scan.current() != "'":
        scan.advance()
      if scan.has_more():
        scan.advance()
      return (True, scan.position())
    return (False, None)


class Scanner(object):
  
  def __init__(self, source):
    self.source = source
    self.pos = 0
  
  def reset(self, pos):
    self.pos = pos
  
  def position(self):
    return self.pos
  
  def has_more(self):
    return self.pos < len(self.source) and self.source[self.pos] != '\n'
  
  def advance(self):
    self.pos += 1
  
  def get_substring(self, start, end):
    return self.source[start:end]
  
  def current(self):
    if self.has_more():
      return self.source[self.pos]
    else:
      return None


class Rule(object):
  
  def __init__(self, parts, output):
    self.parts = parts
    self.output = output
  
  def replace_part(self, index, scan, captures):
    if index == len(self.parts):
      return True
    part = self.parts[index]
    start = scan.position()
    (matched, continue_from) = part.match(scan)
    while matched:
      if self.replace_part(index + 1, scan, captures):
        key = part.get_key()
        if not key is None:
          captures[key] = scan.get_substring(start, continue_from)
        return True
      elif continue_from is None:
        return False
      else:
        scan.reset(continue_from)
        (matched, continue_from) = part.match(scan)
    return False
  
  def replace(self, scan):
    captures = { }
    if self.replace_part(0, scan, captures):
      return captures
    else:
      return None


kPartPattern = re.compile(r'\$\{\s*(\w+)\s*:\s*(\w+)\s*\}')
kPlaceholderPattern = re.compile(r'\$\{\s*(\w+)\s*\}')
class PollockProcessor(object):

  def __init__(self, files):
    self.files = files
    self.rules = { }
    self.matcher = None

  def get_files(self):
    return self.files

  def add_rule(self, pattern, output):
    keyword = pattern[0]
    components = [ ]
    for part in pattern[1:]:
      part_match = kPartPattern.match(part)
      if part_match:
        name = part_match.group(1)
        kind = part_match.group(2)
        if kind == 'expr':
          components.append(Expression(name))
        else:
          raise "Unknown matcher '%s'" % kind
      else:
        components.append(Token(part))
    output = kPlaceholderPattern.sub(r'%(\1)s', output)
    rule = Rule(components, output)
    if not keyword in self.rules:
      self.rules[keyword] = [ ]
    self.rules[keyword].append(rule)

  def loaded(self):
    keys = self.rules.keys()
    self.matcher = re.compile(r'\b(%s)\b' % '|'.join(keys))

  def process_source(self, source, vars):
    last = 0
    match = self.matcher.search(source, last)
    parts = [ ]
    while match:
      if source[match.start()-8:match.start()-1] == '#define':
        match = self.matcher.search(source, match.end())
      else:
        parts.append(source[last:match.start()])
        rules = self.rules[match.group(1)]
        had_match = False
        scanner = Scanner(source)
        for rule in rules:
          scanner.reset(match.end())
          captures = rule.replace(scanner)
          if captures:
            captures.update(**vars)
            output = rule.output % captures
            parts.append(output)
            had_match = True
            break
        if not had_match:
          raise "Bad match '%s'" % match.group(1)
        last = scanner.position()
        match = self.matcher.search(source, last)
    parts.append(source[last:])
    return ''.join(parts)

  def process(self, path, source, vars):
    return """\
#line 1 "%(path)s"
%(source)s""" % {
      'path': path,
      'source': self.process_source(source, vars)
    }


def read_processor(path):
  files = [ join(path, f) for f in listdir(path) if f.endswith('.pollock') ]
  result = PollockProcessor(files)
  for src in files:
    case = None
    for line in open(join(path, src)):
      line = line.strip()
      if not line:
        continue
      if line.startswith('case:'):
        case = line[5:].split()
      elif line.startswith('emit:'):
        output = line[5:].strip()
        result.add_rule(case, output)
      else:
        raise "Malformed input: '%s'" % line
  result.loaded()
  return result
