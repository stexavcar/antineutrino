import types
from SCons.Script import Split, File

# Reads a .list file into an array of strings
def read_list_file(name):
  raw_source = File(name).get_contents()
  lines = raw_source.split("\n")
  list = []
  for line in raw_source.split("\n"):
  	trimmed = line.strip()
  	if len(trimmed) == 0: continue
  	if trimmed[0] == '#': continue
  	list.append(trimmed)
  return list

# Parses a configuration value into a python data structure
def parse_value(str):
  str = str.strip()
  if str.startswith('[') and str.endswith(']'):
    return Split(str.lstrip('[').rstrip(']'))
  else:
    return str

# Apply a set of items read from a configuration file to an
# environment
def apply_items(env, items):
  for key, value in items:
    key = key.upper()
    append = False
    if value.startswith('+'):
      append = True
      value = value.lstrip('+ \t')
    value = parse_value(value)
    if type(value) is types.StringType: value = ' ' + value
    if append: env.Append(**{key: value})
    else: env.Replace(**{key: value})
