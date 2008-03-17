import re
import string
import codecs

def new_namespace(values = None):
  if values is None:
    values = { }
  class Namespace(object):
    def __getattr__(self, key):
      return values[key]
    def __getitem__(self, key):
      return values[key]
    def __str__(self):
      return "namespace %s" % values
    def __repr__(self):
      return str(self)
  return (values, Namespace())

# --- I n i t i a l i z a t i o n ---


(__values_map, values) = new_namespace()
(__lower_to_upper_map, lower_to_upper) = new_namespace()
(__upper_to_lower_map, upper_to_lower) = new_namespace()
def install_value_type(n, NAME, Name, name):
  (_, __values_map[Name]) = new_namespace({
    'index': int(n),
    'NAME': NAME,
    'Name': Name,
    'name': name
  })
  __lower_to_upper_map[name] = Name
  __upper_to_lower_map[Name] = name


(__image_map, fields) = new_namespace()
def install_image_entry(n, Name, Field):
  key = "Image%s_%s" % (Name, Field)
  __image_map[key] = int(n)


(__roots_map, roots) = new_namespace()
def install_root(n, Type, name, Protocol, allocator):
  (_, __roots_map[name]) = new_namespace({
    'index': int(n),
    'type': Type,
    'name': name,
    'protocol': Protocol
  })


def get_string_contents(str):
  (result, _) = codecs.utf_8_decode(str[1:-1])
  return result


(__builtin_functions_map, builtin_functions) = new_namespace()
def install_builtin_function(n, function_name, string_name):
  name = get_string_contents(string_name)
  (_, __builtin_functions_map[name]) = new_namespace({
    'index': int(n)
  })


def install_builtin_method(n, klass, function_name, string_name):
  method_name = get_string_contents(string_name)
  Class = lower_to_upper[klass]
  name = "%s.%s" % (Class, method_name)
  (_, __builtin_functions_map[name]) = new_namespace({
    'index': int(n),
  })


def initialize_from(name):
  consts = read_consts(name)
  consts['FOR_EACH_DECLARED_TYPE'](install_value_type)
  consts['FOR_EACH_IMAGE_OBJECT_CONST'](install_image_entry)
  consts['FOR_EACH_ROOT'](install_root)
  consts['FOR_EACH_BUILTIN_FUNCTION'](install_builtin_function)
  consts['FOR_EACH_BUILTIN_METHOD'](install_builtin_method)


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
