import types, re, sys
from os.path import exists, join

# Reads a .list file into an array of strings
def read_lines_from(name):
  list = []
  for line in open(name):
    trimmed = line[:line.find('#')].strip()
    if len(trimmed) == 0: continue
    list.append(trimmed)
  return list

def escape_string(s):
  s = s.replace('\\', '\\\\')
  return s

def generate_possibilities(params, config):
  options = [ ]
  count = 1
  # Create the list of possible values for each position.  Also, count
  # the number of combinations
  for param in params:
    if not param in config:
      raise "The current configuration does not define parameter '" + param + "'"
    value = config[param]
    if not type(value) is list: value = [ value ]
    options.append(value)
    count *= len(value)
  # If this assert fails you're either being too stupid or too clever
  # with the build files.  Don't do that.
  assert count < 100
  # Generate all combination by encoding the index of each combination
  # as a mixed-radix decimal representation
  current = [ 0 ] * len(params)
  combinations = [ ]
  for i in xrange(0, count):
    combination = [ ]
    for j in xrange(0, len(options)):
      combination.append(i % len(options[j]))
      i = i / len(options[j])
    combinations.append(combination)
  # Compute the distance from the given configuration to being a
  # perfect match
  def dist(value):
    result = 0
    for elm in value:
      result += (elm * elm)
    return result
  # Sort the vectors the the better matches come first (as determined
  # by their distance as computed by dist) and elements at the same
  # distance are lexicographically sorted
  def compare_vectors(a, b):
    a_dist = dist(a)
    b_dist = dist(b)
    if (a_dist == b_dist):
      for i in xrange(0, len(options)):
        if a[i] == b[i]: continue
        elif a[i] > b[i]: return 1
        else: return -1
      return 0
    elif a_dist > b_dist: return 1
    else: return -1
  combinations.sort(cmp=compare_vectors)
  # Create a map for each combination that gives the mapping to be
  # performed on the template string
  result = [ ]
  for combination in combinations:
    map = { }
    for i in xrange(0, len(params)):
      map[params[i]] = options[i][combination[i]].lower()
    result.append(map)
  return result

# Read a list of files from a file and expand wildcards according to
# the current plaform, as specified by the configuration
def read_files_from(config, root, file):
  cooked_lines = []
  for line in read_lines_from(file):
    if exists(join(root, line)):
      # If the file exists we just add it to the list
      cooked_lines.append(line)
    else:
      # If the file doesn't exist we try expanding parameters
      params = re.findall('<([a-z]+)>', line)
      params.sort()
      possibilities = generate_possibilities(params, config)
      template = re.sub('<([a-z]+)>', '%(\\1)s', line)
      found_match = False
      for map in possibilities:
        specific = template % map
        if exists(join(root, specific)):
          # A file exists for this particular platform; add it
          cooked_lines.append(specific)
          found_match = True
          break
      if not found_match:
        cooked_lines.append(line)
  return cooked_lines

HEADER_PATTERN = re.compile('^(\w+)\s*(\*?)\s*(?:\[\s*(\w*)\s*\])?\s*(?:\:\s*([\w\s]+))?{$')
PROPERTY_PATTERN = re.compile('^(\w+)\s*(\:\=|\+\=)(.*)$')

class Configuration:
  def __init__(self, sections, targets, options):
    self.sections = sections
    self.targets = targets
    self.options = options
  def __repr__(self):
    return 'config ' + str(self.sections)
  def get_properties(self, name, options):
    properties = { }
    self.load_section_into(self.sections[name], properties)
    for (name, value) in options.items():
      self.load_section_into(self.options[name].values[value], properties)
    return properties
  def load_section_into(self, section, properties):
    for super in section.supers:
      self.load_section_into(self.sections[super], properties)
    for (key, (value, is_extension)) in section.properties.items():
      if (key in properties) and is_extension:
        old_value = properties[key]
        if type(old_value) is str:
          value = old_value + ' ' + value
        elif type(old_value) is list:
          value = old_value + value
        else:
          assert False
      properties[key] = value

class Section:
  def __init__(self, name, supers, properties):
    self.name = name
    self.supers = supers
    self.properties = properties
  def __repr__(self):
    return 'section ' + str(self.properties)

class Option:
  def __init__(self, name):
    self.name = name
    self.values = { }
    self.default = None

def read_config_file(name, parent=None):
  lines = read_lines_from(name)
  # Mapping from section names to a mapping from property name to a
  # pair of the value plus whether or not it is an extension
  sections = { }
  # A list of names of targets
  targets = [ ]
  # The set of available options
  options = { }
  if parent:
    sections.update(**parent.sections)
    targets += parent.targets
    options.update(**parent.options)
  index = 0
  # Function used to signal when an unexpected line occurs
  def unexpected_line():
    print "Unexpected line '" + lines[index] + "'"
    sys.exit(1)
  # Transforms a string value into a structured value if necessary
  def parse_value(str):
    if str.startswith('[') and str.endswith(']'):
      return str[1:-1].split()
    else:
      return str
  while index < len(lines):
    header_match = HEADER_PATTERN.match(lines[index])
    if not header_match: unexpected_line()
    index = index + 1
    section = header_match.group(1)
    option_name = (header_match.group(3))
    is_target = (header_match.group(2) == '*')
    if is_target and not option_name: targets.append(section)
    supers_str = header_match.group(4)
    if supers_str: supers = supers_str.split()
    else: supers = []
    # Read the lines following the section header and load them into
    # the properties mapping
    properties = { }
    if section in sections:
      previous = sections[section]
      properties.update(**previous.properties)
      supers = previous.supers + supers
    # Sets a name to a value using the specified mode
    # Then add the lines read from this section
    while lines[index] != '}':
      property_match = PROPERTY_PATTERN.match(lines[index])
      if not property_match: unexpected_line()
      name = property_match.group(1)
      operator = property_match.group(2)
      value = parse_value(property_match.group(3).strip())
      properties[name] = (value, operator == '+=')
      index = index + 1
    obj = Section(section, supers, properties)
    sections[section] = obj
    if option_name:
      if not option_name in options:
        options[option_name] = Option(option_name)
      option = options[option_name]
      option.values[section] = obj
      if is_target: option.default = section
    index = index + 1
  return Configuration(sections, targets, options)

# Apply a set of items read from a configuration file to an
# environment
def apply_items(env, properties):
  for (key, value) in properties.items():
    env.Replace(**{key.upper(): value})

KEYWORDS = [
  'def', 'this', 'is', 'if', 'else', 'while', 'do', 'return', 'null',
  'true', 'false', 'internal', 'operator', 'new', 'in', 'fn', 'and',
  'or', 'not', 'on', 'raise', 'protocol', 'native'
]
