from os import listdir
from os.path import join, abspath
import re


class Rule(object):
  
  def __init__(self, punctuation, placeholders, output):
    self.punctuation = punctuation
    self.placeholders = placeholders
    self.output = output
  
  def replace(self, source):
    parts = split_by_punctuation(source, self.punctuation)
    if not parts:
      return None
    captures = { }
    for (name, index) in self.placeholders:
      captures[name] = parts[index]
    return captures


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
    output = kPlaceholderPattern.sub(r'%(\1)s', output)
    rule = Rule(punctuation, placeholders, output)
    if not keyword in self.rules:
      self.rules[keyword] = [ ]
    self.rules[keyword].append(rule)

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
            output = rule.output % captures
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
      'path': path,
      'source': self.process_source(source, path, vars)
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
