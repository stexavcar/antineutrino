import os
import re
import sys
from os.path import abspath, join, dirname, commonprefix
from utils import read_lines_from


SECTION_BEGIN = re.compile(r"\s*(\w+)\s*\{")
SECTION_END = re.compile(r"}")
REQUIRE_DIRECTIVE = re.compile(r"require\s*(.*)")
INCLUDE_DIRECTIVE = re.compile(r"include\s*(.*)")
DEFINE_DIRECTIVE = re.compile(r"(\w+)\s*([:+-])=\s*(.*)")
OPTION_DIRECTIVE = re.compile(r"option\s*(\w+)\s*:(.*)")
USE_DIRECTIVE = re.compile(r"use\s*(\w+)")


HEADER = re.compile(r"\[\s*(\w+)\s*(?:\:(.*))?\s*\]")


class Section(object):

  def __init__(self, name):
    self.name = name
    self.used = [ ]
    self.definitions = [ ]
    self.requirements = [ ]
    self.included = [ ]

  def specialize(self, top, settings, result, files):
    for req in self.requirements:
      if not req.evaluate(settings):
        return
    for used in self.used:
      top.use(used, top, settings, result, files)
    for included in self.included:
      config = files.load_config([included], None)
      config.do_specialize(top, settings, result, files)
    for (name, op, value) in self.definitions:
      current = [ ]
      if op != ':':
        current = result.get(name, current)
      if op == '-':
        current = [ x for x in current if not x in value ]
      else:
        current += value
      result[name] = current

  def add_use(self, name):
    self.used.append(name)

  def add_definition(self, name, op, value):
    assert type(value) is list
    self.definitions.append((name, op, value))

  def add_requirement(self, expr):
    self.requirements.append(expr)

  def add_include(self, path):
    self.included.append(path)


class Configuration(object):

  def __init__(self, parent):
    self.parent = parent
    self.sections = [ ]
    self.options = { }

  def use(self, name, top, settings, result, files):
    if self.parent:
      self.parent.use(name, top, settings, result, files)
    for section in self.sections:
      if section.name == name:
        section.specialize(top, settings, result, files)

  def specialize(self, top, settings, files):
    result = { }
    self.do_specialize(top, settings, result, files)
    return result

  def do_specialize(self, top, settings, result, files):
    if self.parent:
      self.parent.do_specialize(top, settings, result, files)
    for section in self.sections:
      section.specialize(top, settings, result, files)

  def add_section(self, section):
    self.sections.append(section)


def adjust_file_name(s):
  return s.replace('/', os.path.sep)


def read_sections_from(file):
  last_header = None
  last_entries = [ ]
  result = { }
  for line in read_lines_from(file):
    header_match = HEADER.match(line)
    if header_match:
      if last_header:
        result[last_header] = last_entries
      last_header = header_match.group(1)
      parents = header_match.group(2)
      last_entries = [ ]
      if parents:
        for parent in [ p.strip() for p in parents.split(',') ]:
          last_entries += result[parent]
      continue
    else:
      last_entries.append(adjust_file_name(line))
  if last_header:
    result[last_header] = last_entries
  return result


def esc(s):
  s = s.replace('\\', '\\\\')
  s = s.replace('|', '\\')
  s = s.replace('.', '\\.')
  return s


class InputFileSet(object):

  def __init__(self, lists, variables):
    self.lists = lists
    self.master_config_ = { }
    self.abstract_files = { }
    self.config_cache = { }
    self.variables = variables

  def load(self, master_config):
    self.master_config_ = self.load_config([master_config], None)
    for l in self.lists:
      self.resolve_file_list(l)

  def resolve_file_list(self, list):
    """Iterates through the specified list and finds the associated files
       by specializing the wildcards according to the current platform.
    """
    # First build a list of all files in the directory that holds the file
    # list, recursively
    root = dirname(list)
    all_files = [ ]
    for (path, dirs, files) in os.walk(root):
      all_files += [ join(path, f) for f in files ]
    # Then read the file list and use it to filter the list of all files
    for (section, entries) in read_sections_from(list).items():
      file_list = [ ]
      for entry in entries:
        pattern_str = esc(join(root, re.sub(r"<(\w+)>", r"(?P<\1>|w+)", entry)))
        pattern = re.compile(pattern_str)
        matches = [ ]
        for f in all_files:
          match = pattern.match(f)
          if match:
            wildcards = match.groupdict()
            name = match.group(0)
            matches.append((name, wildcards))
        if not matches:
          raise "Found no file matching %s" % entry
        file_list.append(AbstractFile(matches, root))
      self.abstract_files[section] = file_list

  def sub_variable(self, match):
    return self.variables[match.group(1).lower()]

  def process_value(self, value):
    parts = value.split()
    return [ re.sub(r'%\{(\w+)\}', self.sub_variable, p) for p in parts ]

  def process_lines(self, lines, root, config):
    current_section = None
    for line in lines:
      line = line.strip()
      section_begin_match = SECTION_BEGIN.match(line)
      if section_begin_match:
        current_section = Section(section_begin_match.group(1))
        continue
      section_end_match = SECTION_END.match(line)
      if section_end_match:
        config.add_section(current_section)
        current_section = None
        continue
      option_match = OPTION_DIRECTIVE.match(line)
      if option_match:
        name = option_match.group(1)
        raw_values = [ o.strip() for o in option_match.group(2).split(',') ]
        defaults = [ o[1:] for o in raw_values if o.startswith('*') ]
        non_defaults = [ o for o in raw_values if not o.startswith('*') ]
        if len(defaults) == 1:
          default = defaults[0]
        else:
          default = None
        config.options[name] = (defaults + non_defaults, default)
        continue
      require_match = REQUIRE_DIRECTIVE.match(line)
      if require_match:
        requirement = self.parse_expression(require_match.group(1))
        current_section.add_requirement(requirement)
        continue
      include_match = INCLUDE_DIRECTIVE.match(line)
      if include_match:
        included = include_match.group(1)
        path = join(root, included)
        current_section.add_include(path)
        continue
      define_match = DEFINE_DIRECTIVE.match(line)
      if define_match:
        name = define_match.group(1)
        op = define_match.group(2)
        value_str = define_match.group(3)
        value = self.process_value(value_str)
        current_section.add_definition(name, op, value)
        continue
      use_match = USE_DIRECTIVE.match(line)
      if use_match:
        used = use_match.group(1)
        current_section.add_use(used)
        continue
      raise "Unexpected line '%s'" % line

  def parse_expression(self, expr):
    tokens = TokenStream(expr.split())
    return self.parse_binary_expression(tokens)

  def parse_binary_expression(self, tokens):
    left = self.parse_atomic_expression(tokens)
    if tokens.has_more():
      op = tokens.current()
      tokens.advance()
      right = self.parse_atomic_expression(tokens)
      return Operator(left, op, right)
    else:
      return left

  def parse_atomic_expression(self, tokens):
    current = tokens.current()
    if current.startswith('$'):
      tokens.advance()
      return Variable(current[1:])
    elif current.startswith("'"):
      tokens.advance()
      return Literal(current[1:-1])
    elif current == '(':
      tokens.advance()
      expr = self.parse_binary_expression(tokens)
      assert tokens.current() == ')'
      tokens.advance()
      return expr
    else:
      raise "Unexpected token '%s'" % current

  def load_config(self, files, parent):
    if not files: return parent
    if not tuple(files) in self.config_cache:
      config = Configuration(parent)
      for full_file in files:
        lines = read_lines_from(full_file)
        self.process_lines(lines, dirname(full_file), config)
      self.config_cache[tuple(files)] = config
    return self.config_cache[tuple(files)]

  def get_configured_files(self, key, settings):
    result = [ ]
    for abf in self.abstract_files[key]:
      path = abf.specialize(settings)
      configuration = self.get_flags_for_file(path, abf.root)
      if configuration:
        flags = configuration.specialize(configuration, settings, self)
      else:
        flags = { }
      relpath = path[len(commonprefix([path, abf.root]))+1:]
      result.append(ConfiguredFile(relpath, flags))
    return result

  def get_flags_for_dir(self, path, root):
    if not path in self.config_cache:
      if path == root:
        parent = None
      else:
        parent = self.get_flags_for_dir(dirname(path), root)
      files = [ join(path, f) for f in os.listdir(path) if f.endswith('.cfg') ]
      self.config_cache[path] = self.load_config(files, parent)
    return self.config_cache[path]

  def get_flags_for_file(self, filename, root):
    parent = dirname(filename)
    return self.get_flags_for_dir(parent, root)

  def options(self):
    return self.master_config_.options

  def get_master_flags(self, settings):
    return self.master_config_.specialize(self.master_config_, settings, self)


# Guard expression parsing


class TokenStream(object):

  def __init__(self, tokens):
    self.tokens = tokens
    self.pos = 0

  def current(self):
    return self.tokens[self.pos]

  def advance(self):
    self.pos += 1

  def has_more(self):
    return self.pos < len(self.tokens)


class Operator(object):

  def __init__(self, left, op, right):
    self.left = left
    self.op = op
    self.right = right

  def evaluate(self, settings):
    left = self.left.evaluate(settings)
    right = self.right.evaluate(settings)
    if self.op == '=':
      return [ e for e in left if e in right ]
    elif self.op == 'or':
      return left or right
    else:
      raise "Unknown guard operator '%s'" % self.op

class Variable(object):

  def __init__(self, name):
    self.name = name

  def evaluate(self, settings):
    return settings.get(self.name, [ ])


class Literal(object):

  def __init__(self, value):
    self.value = value

  def evaluate(self, settings):
    return [ self.value ]


class AbstractFile(object):
  """A set of source files, each associated with some mode, platform, or
  similar.  Once the current settings have been determined one of these
  source files will be selected as the most specific one by the specialize
  method.
  """

  def __init__(self, names, root):
    self.names = names
    self.root = root

  def pick_best(self, a, b, settings):
    """Returns true if 'a' is a better or more specific configuration
    than 'b'.
    """
    (name_a, config_a) = a
    (name_b, config_b) = b
    for (key, value) in config_a.items():
      if not key in config_b: return a
      if not key in settings: return a
      sequence = settings[key]
      if not value in sequence: return b
      if not config_b[key] in sequence: return a
      if settings[key].index(value) > settings[key].index(config_b[key]):
        return b
      else:
        return a
    return b

  def specialize(self, settings):
    """Returns the most specific file of this file set, base on the current
    settings as specified through the settings map.
    """
    current_best = self.names[0]
    for i in xrange(1, len(self.names)):
      current_best = self.pick_best(current_best, self.names[i], settings)
    return current_best[0]


class ConfiguredFile(object):

  def __init__(self, path, flags):
    self.path = path
    self.flags = flags


def build_configuration(root, lists, variables, master_config):
  input_set = InputFileSet(lists, variables)
  input_set.load(master_config)
  return input_set
