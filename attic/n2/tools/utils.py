import platform
import types, re, sys
from os.path import exists, join

# Reads a .list file into an array of strings
def read_lines_from(name):
  list = []
  for line in open(name):
    if '#' in line:
      line = line[:line.find('#')]
    line = line.strip()
    if len(line) == 0: continue
    list.append(line)
  return list

def escape_string(s):
  s = s.replace('\\', '\\\\')
  return s

# Apply a set of items read from a configuration file to an
# environment
def apply_items(env, properties):
  for (key, (value, append)) in properties.items():
    if append: env.Append(**{key.upper(): value})
    else: env.Replace(**{key.upper(): value})


def guess_toolchain():
  return 'gcc'


def guess_os():
  system = platform.system()
  if system == 'Darwin':
    return 'darwin'
  elif system == 'Windows':
    return 'win32'
  else:
    return 'linux'
