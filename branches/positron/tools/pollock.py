from os import listdir
from os.path import join, abspath
import re


class Rule(object):
  
  def __init__(self, punctuation, placeholders):
    self.punctuation = punctuation
    self.placeholders = placeholders
  
  def replace(self, source):
    parts = split_by_punctuation(source, self.punctuation)
    if not parts:
      return None
    captures = { }
    for (name, index) in self.placeholders:
      captures[name] = parts[index]
    return captures


class SimpleRule(Rule):

  def __init__(self, punctuation, placeholders, output):
    super(SimpleRule, self).__init__(punctuation, placeholders)
    self.output = output
  
  def emit(self, vars):
    return self.output % vars


class PyRule(Rule):

  def __init__(self, punctuation, placeholders, expander):
    super(PyRule, self).__init__(punctuation, placeholders)
    self.expander = expander
  
  def emit(self, vars):
    return self.expander(**vars)


def escape(str):
  return str.replace('\\', '\\\\')


kPartPattern = re.compile(r'\$\{\s*(\w+)\s*:\s*(\w+)\s*\}')
kPlaceholderPattern = re.compile(r'\$\{\s*(\w+)\s*\}')
class PollockProcessor(object):

  def __init__(self, files):
    self.files = files
    self.rules = { }
    self.matcher = None

  def get_files(self):
    return self.files

  def split_pattern(self, pattern):
    keyword = pattern[0]
    placeholders = [ ]
    punctuation = [ ]
    for part in pattern[1:]:
      part_match = kPartPattern.match(part)
      if part_match:
        name = part_match.group(1)
        kind = part_match.group(2)
        if kind == 'expr':
          placeholders.append((name, len(punctuation)))
        else:
          raise "Unknown matcher '%s'" % kind
      else:
        punctuation.append(part)
    return (keyword, punctuation, placeholders)

  def add_rule(self, keyword, rule):
    if not keyword in self.rules:
      self.rules[keyword] = [ ]
    self.rules[keyword].append(rule)

  def add_simple_rule(self, pattern, output):
    (keyword, punctuation, placeholders) = self.split_pattern(pattern)
    output = kPlaceholderPattern.sub(r'%(\1)s', output)
    rule = SimpleRule(punctuation, placeholders, output)
    self.add_rule(keyword, rule)
  
  def add_py_rule(self, pattern, params, lines):
    (keyword, punctuation, placeholders) = self.split_pattern(pattern)
    namespace = { }
    source = "def expander(%(params)s):\n%(body)s" % {
      'params': ",".join(params + ["**rest"]),
      'body': "".join(lines)
    }
    exec source in namespace
    expander = namespace['expander']
    rule = PyRule(punctuation, placeholders, expander)
    self.add_rule(keyword, rule)

  def loaded(self):
    keys = self.rules.keys()
    self.matcher = re.compile(r'\b(%s)\b([^;]*;)' % '|'.join(keys))

  def report_error(self, source, name, offset, tag):
    line = len(source[:offset].split("\n"))
    print "%s:%i: Malformed '%s'." % (name, line, tag)
    raise AssertionError()

  def process_source(self, source, name, vars):
    last = 0
    match = self.matcher.search(source)
    parts = [ ]
    while match:
      if False and source[match.start()-8:match.start()-1] == '#define':
        match = self.matcher.search(source, match.end())
      else:
        parts.append(source[last:match.start()])
        rules = self.rules[match.group(1)]
        had_match = False
        offset = None
        for rule in rules:
          captures = rule.replace(match.group(2))
          if captures:
            captures.update(**vars)
            output = rule.emit(captures)
            parts.append(output)
            had_match = True
            offset = match.end(2)
            break
        if not had_match:
          self.report_error(source, name, match.start(0), match.group(1))
        last = offset
        match = self.matcher.search(source, last)
    parts.append(source[last:])
    return ''.join(parts)

  def process(self, path, source, vars):
    return """\
#line 1 "%(path)s"
%(source)s""" % {
      'path': escape(path),
      'source': self.process_source(source, path, vars)
    }


kPyEmitPattern = re.compile(r'emit\(([^)]*)\):')
def read_processor(path):
  files = [ join(path, f) for f in listdir(path) if f.endswith('.pollock') ]
  result = PollockProcessor(files)
  params = None
  lines = [ ]
  for src in files:
    case = None
    for raw_line in open(join(path, src)):
      line = raw_line.strip()
      if not line:
        if params:
          result.add_py_rule(case, params, lines)
        params = None
        continue
      if params:
        lines.append(raw_line)
        continue
      if line.startswith('case:'):
        case = line[5:].split()
      elif line.startswith('emit:'):
        output = line[5:].strip()
        result.add_simple_rule(case, output)
      elif line.startswith('emit('):
        match = kPyEmitPattern.match(line)
        params = [ s.strip() for s in match.group(1).split(',')]
      else:
        raise "Malformed input: '%s'" % line
  if params:
    result.add_py_rule(case, params, lines)
  result.loaded()
  return result


def count_regexp(str, regex):
  count = 0
  for occ in regex.finditer(str):
    count += 1
  return count


kStartPattern = re.compile(r'[\(\[\{]')
kEndPattern = re.compile(r'[\)\]\}]')
def chunk_height(chunk):
  return count_regexp(chunk, kStartPattern) - count_regexp(chunk, kEndPattern)


def split_by_punctuation(input, punctuation):
  current_offset = 0
  parts = [ ]
  for elm in punctuation:
    height = 0
    chunks = [ ]
    while True:
      next_offset = input.find(elm, current_offset)
      if next_offset == -1:
        return None
      chunk = input[current_offset:next_offset]
      height += chunk_height(chunk)
      chunks.append(chunk)
      current_offset = next_offset + len(elm)
      if height == 0:
        break
      else:
        chunks.append(elm)
    parts.append("".join(chunks).strip())
  return parts


def test_split(input, punctuation, expected):
  (value, offset) = split_by_punctuation(input, punctuation)
  if value != expected:
    raise "Expected: <%s>\nFound: <%s>" % (expected, value)


def test():
  punctuation = ['*', '$']
  test_split("foo * bar $", punctuation, ['foo', 'bar'])
  test_split("foo * bar *", punctuation, None)
  test_split("foo * (bar $ baz) $", punctuation, ['foo', '(bar $ baz)'])
  test_split("foo * {bar $ baz} $", punctuation, ['foo', '{bar $ baz}'])
  test_split("foo * [bar $ baz] $", punctuation, ['foo', '[bar $ baz]'])
  wide_punct = ['=>', ';;']
  test_split("foo=>bar;;", wide_punct, ['foo', 'bar'])


if __name__ == '__main__':
  test()
