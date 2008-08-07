import re
import os
from os.path import join


LINE_HEADER = r'^def\s+@(\w+)\s*(\([^)]*\))?\s*(.*)$'
GROUP = r'^\$\{\s*(\w+)\s*:(.*)\}$'
MATCHERS = {
  'ident': r'[\w_]+',
  'expr': r'[^;]+?',
  'relation': r'(?:==|<=|>=|<|>|!=)'
}


def pattern_to_regexp(parts):
  result = [ ]
  for part in parts:
    group_match = re.match(GROUP, part)
    if group_match:
      name = group_match.group(1)
      body = group_match.group(2)
      part = "(?P<%s>%s)" % (name, MATCHERS[body])
    result.append(part)
  return "^\s*" + "\s*".join(result) + "\s*$"

def to_template(str):
  return re.sub(r'\$\{\s*(\w+)\s*\}', '%(\g<1>)s', str)

def lines_to_template(lines):
  block = " ".join([ line.strip() for line in lines ])
  block = block.replace('%', '%%')
  block = to_template(block)
  return block

def parse_params(match):
  if not match: return []
  match = match[1:-1]
  match = match.split(',')
  return [ (k.strip(), to_template(v)) for (k, v) in [ s.split('=') for s in match ] ]

def parse_args(match):
  if not match: return []
  match = match[1:-1]
  return [ s.strip() for s in match.split(',') ]

def make_specificiation(dir):
  files = [ f for f in os.listdir(dir) if f.endswith('.pollock') ]
  tags = { }
  for f in files:
    read_specification(open(join(dir, f), "rt").read(), tags)
  return PollockProcessor(tags)

def read_specification(data, tags):
  last_header_match = None
  lines = []
  def process_section():
    if not last_header_match: return
    tag = last_header_match.group(1)
    params = parse_params(last_header_match.group(2))
    pattern = pattern_to_regexp(last_header_match.group(3).split())
    replacement = lines_to_template(lines)
    if not tag in tags: tags[tag] = []
    tags[tag].append(Matcher(pattern, params, replacement))
  for line in data.split("\n"):
    if '#' in line: line = line[:line.index('#')]
    line = line.strip()
    if not line: continue
    header_match = re.match(LINE_HEADER, line)
    if header_match:
      process_section()
      lines = []
      last_header_match = header_match
    else:
      lines.append(line)
  process_section()

  
ANNOTATED_STATEMENT = r'(\n[ \t]*)@(\w+)\s*(\([^)]*\))?\s*([^;]*);(?=\n)'


class Matcher(object):

  def __init__(self, pattern, params, value):
    self.pattern = pattern
    self.params = params
    self.value = value
  
  def replace(self, input, args, counter):
    match = re.match(self.pattern, input)
    if match:
      dict = match.groupdict()
      dict['i'] = str(counter)
      for i in xrange(len(self.params)):
        (key, value) = self.params[i]
        if i < len(args):
          dict[key] = args[i]
        else:
          dict[key] = value % dict
      return self.value % dict
    else:
      return None


class Counter(object):

  def __init__(self):
    self.index = 0
  
  def next(self):
    value = self.index
    self.index += 1
    return value


def escape(s):
  return s.replace('\\', '\\\\')


class PollockProcessor(object):

  def __init__(self, matchers):
    self.matchers = matchers

  def process(self, name, str):
    counter = Counter()
    str = re.sub(ANNOTATED_STATEMENT, lambda x: self.process_match(x, counter), str)
    return ('#line %i "%s" \n' % (1, escape(name))) + str
  
  def process_match(self, match, counter):
    preamble = match.group(1)
    tag = match.group(2)
    args = parse_args(match.group(3))
    stmt = match.group(4)
    list = self.matchers[tag]
    index = counter.next()
    for matcher in list:
      replacement = matcher.replace(stmt, args, index)
      if replacement:
        return preamble + replacement + ';'
    return preamble + stmt + ';'
