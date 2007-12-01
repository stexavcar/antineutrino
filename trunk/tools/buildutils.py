import types, re, sys

# Reads a .list file into an array of strings
def read_lines_from(name):
  raw_source = open(name).read()
  lines = raw_source.split("\n")
  list = []
  for line in raw_source.split("\n"):
  	trimmed = line.strip()
  	if len(trimmed) == 0: continue
  	if trimmed[0] == '#': continue
  	list.append(trimmed)
  return list

HEADER_PATTERN = re.compile('^(\w+)\s*(\*?)\s*(?:\:\s*([\w\s]+))?{$')
PROPERTY_PATTERN = re.compile('^(\w+)\s*(\:\=|\+\=)(.*)$')

def read_config_file(name):
  lines = read_lines_from(name)
  # Mapping from section names to a mapping from property name to a
  # pair of the value plus whether or not it is an extension
  sections = { }
  # A list of names of targets
  targets = [ ]
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
    section = header_match.group(1)
    is_target = (header_match.group(2) == '*')
    if is_target: targets.append(section)
    supers = header_match.group(3)
    # Read the lines following the section header and load them into
    # the properties mapping
    properties = { }
    # Sets a name to a value using the specified mode
    def apply_property(name, value, is_extension):
      if is_extension:
        if name in properties:
          (old_value, is_extension) = properties[name]
          if type(old_value) is str:
            value = old_value + ' ' + value
          elif type(old_value) is list:
            value = old_value + value
          else:
            assert False
        properties[name] = (value, True)
      else:
        properties[name] = (value, False)
    # If there are super sections we run through them and add them
    # to the mapping for this section
    if supers:
      for super in supers.split():
        for (name, (value, is_extension)) in sections[super].items():
          apply_property(name, value, is_extension)
    index = index + 1
    # Then add the lines read from this section
    while lines[index] != '}':
      property_match = PROPERTY_PATTERN.match(lines[index])
      if not property_match: unexpected_line()
      name = property_match.group(1)
      operator = property_match.group(2)
      value = parse_value(property_match.group(3).strip())
      apply_property(name, value, operator == '+=')
      index = index + 1
    sections[section] = properties
    index = index + 1
  # Finally, we reduce the information to a mapping just containing
  # the targets and with not information about whether or not 
  # properties were extensions or not
  result = { }
  for target in targets:
    properties = { }
    for (name, (value, is_extension)) in sections[target].items():
      properties[name] = value
    result[target] = properties
  return result

# Apply a set of items read from a configuration file to an
# environment
def apply_items(env, properties):
  for (key, value) in properties.items():
    env.Replace(**{key.upper(): value})
