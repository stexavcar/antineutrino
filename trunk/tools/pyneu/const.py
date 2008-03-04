import re
import string

# --- I n i t i a l i z a t i o n ---

values = { }

def install_value_type(n, NAME, Name, name):
  values[Name] = { 
    'n': n,
    'NAME': NAME,
    'Name': Name,
    'name': name
  }

def initialize_from(name):
  consts = read_consts(name)
  consts['FOR_EACH_VALUE_TYPE'](install_value_type)

# --- M a c r o   P a r s i n g ---

class MacroCollection(object):

  def __init__(self):
    self.defines = { }

  def add(self, name, value):
    self.defines[name] = value

  def __getitem__(self, name):
    return BoundMacro(self, self.defines[name])


class BoundMacro(object):
  
  def __init__(self, collection, macro):
    self.collection_ = collection
    self.macro_ = macro
  
  def __call__(self, function):
    self.macro_.apply(self.collection_, function)


class Macro:

  def __init__(self, args, body):
    self.args = args
    self.body = body

  def apply(self, macros, function):
    for arg in self.args:
      call_pattern = re.compile(arg + "\(([^)]*)\)")
      calls = call_pattern.findall(self.body)
      for call in calls:
        args = map(string.strip, call.split(","))
        function(*args)
    forward_pattern = re.compile("([A-Za-z0-9_]+)\(" + ", ".join(self.args) + '\)')
    forwards = forward_pattern.findall(self.body)
    for forward in forwards:
      macros[forward](function)


const_pattern = re.compile("^#define ([a-zA-Z0-9_]+)\(([^)]*)\)(.*)$")
def read_consts(file):
  defines = MacroCollection()
  input = open(file).read()
  input = input.replace("\\\n", "")
  lines = input.split("\n")
  for line in lines:
    stripped = line.strip()
    match = const_pattern.match(stripped)
    if not match: continue
    name = match.group(1).strip()
    args = match.group(2).strip().split()
    body = match.group(3).strip()
    defines.add(name, Macro(args, body))
  return defines
