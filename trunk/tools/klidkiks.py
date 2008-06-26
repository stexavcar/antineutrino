import re


LINE_HEADER = r'^def\s+@(\w+)\s*(.*)$'
GROUP = r'^\$\{\s*(\w+)\s*:(.*)\}$'


def pattern_to_regexp(parts):
  result = [ ]
  for part in parts:
    group_match = re.match(GROUP, part)
    if group_match:
      name = group_match.group(1)
      body = group_match.group(2)
      part = "(?P<%s>%s)" % (name, body)
    result.append(part)
  return "^\s*" + "\s*".join(result) + "\s*$"

def lines_to_template(lines):
  block = " ".join([ line.strip() for line in lines ])
  block = block.replace('%', '%%')
  block = re.sub(r'\$\{\s*(\w+)\s*\}', '%(\g<1>)s', block)
  return block

def read_specification(data):
  last_header_match = None
  lines = []
  tags = { }
  def process_section():
    if not last_header_match: return
    tag = last_header_match.group(1)
    pattern = pattern_to_regexp(last_header_match.group(2).split())
    replacement = lines_to_template(lines)
    if not tag in tags: tags[tag] = []
    tags[tag].append(Matcher(pattern, replacement))
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
  return KlidKiksProcessor(tags)

  
ANNOTATED_STATEMENT = r'(\n[ \t]*)@(\w+)\s*([^;]*);(?=\n)'


class Matcher(object):

  def __init__(self, pattern, value):
    self.pattern = pattern
    self.value = value
  
  def replace(self, str):
    match = re.match(self.pattern, str)
    if match: return self.value % match.groupdict()
    else: return None


class KlidKiksProcessor(object):

  def __init__(self, matchers):
    self.matchers = matchers

  def process(self, name, str):
    str = re.sub(ANNOTATED_STATEMENT, lambda x: self.process_match(x), str)
    return ('#line %i "%s" \n' % (1, name)) + str
  
  def process_match(self, match):
    preamble = match.group(1)
    tag = match.group(2)
    stmt = match.group(3)
    list = self.matchers[tag]
    for matcher in list:
      replacement = matcher.replace(stmt)
      if replacement: return preamble + replacement + ';'
    return preamble + stmt + ';'
