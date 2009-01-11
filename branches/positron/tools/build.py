from os.path import dirname, join
import platform
import re


class BuildObject(object):
  """A section in a build configuration"""

  def __init__(self, config, root, args):
    self.config = config
    self.root = root
    self.args = args
  
  def get_name(self):
    return self.args['name']
  
  def get_flags(self):
    flags = self.args.get('flags', None)
    if flags:
      return self.config.flags[flags].to_dict()
    else:
      return { }
  
  def get_dependencies(self):
    result = [ ]
    for entry in self.args['srcs']:
      if entry.startswith('$'):
        result += self.config.vars[entry[1:]]
      else:
        result.append(entry)
    return result
  
  def get_domain(self):
    return self.args.get('domain', None)
  
  def get_libraries(self):
    srcs = self.get_dependencies()
    return [p[1:] for p in srcs if p.startswith(':')]
  
  def get_sources(self):
    srcs = self.get_dependencies()
    return [join(self.root, p) for p in srcs if p.endswith('.cc')]
  
  def get_headers(self):
    srcs = self.get_dependencies()
    return [join(self.root, p) for p in srcs if p.endswith('.h')]


class Library(BuildObject):
  """A libraries section in a build configuration"""
  
  def __init__(self, config, root, args):
    super(Library, self).__init__(config, root, args)


class Program(BuildObject):
  """A program section in a build configuration"""
  
  def __init__(self, config, root, args):
    super(Program, self).__init__(config, root, args)


class Flags(object):

  def __init__(self, config, args):
    self.config = config
    self.args = args
    self.flags = None
  
  def apply_flag_map(self, flags, dict, target):
    if flags is None:
      return
    if type(flags) is str:
      flags = [flags]
    for flag in flags:
      if flag in dict:
        target.append(dict[flag])
  
  def to_dict(self):
    if self.flags:
      return self.flags
    parent = self.args.get('extend', None)
    if parent:
      self.flags = { }
      self.flags.update(**self.config.flags[parent].to_dict())
    else:
      self.flags = {
        'CXXFLAGS': [ '$WARNINGFLAGS' ],
        'WARNINGFLAGS': [ ],
        'CPPDEFINES': [ ]
      }
    self.process_warnings(self.args.get('warnings', []))
    self.process_dialects(self.args.get('dialect', []))
    self.process_defines(self.args.get('defines', []))
    self.process_optimize(self.args.get('optimize', None))
    self.process_debug(self.args.get('debug', 'off'))
    return self.flags


class GccFlags(Flags):

  kDialect = {
    'ansi': '-ansi',
    'pedantic': '-pedantic'
  }
  
  kWarning = {
    'all': '-Wall',
    'no-unused-parameter': '-Wno-unused-parameter',
    'no-non-virtual-dtor': '-Wno-non-virtual-dtor'
  }
  
  kOptimize = {
    'no': '-O0',
    'speed': '-O3'
  }

  def __init__(self, config, args):
    super(GccFlags, self).__init__(config, args)

  def process_dialects(self, dialects):
    self.apply_flag_map(dialects, GccFlags.kDialect, self.flags['CXXFLAGS'])
  
  def process_warnings(self, warnings):
    self.apply_flag_map(warnings, GccFlags.kWarning, self.flags['WARNINGFLAGS'])
  
  def process_defines(self, defines):
    self.flags['CPPDEFINES'] += defines
  
  def process_optimize(self, level):
    self.apply_flag_map(level, GccFlags.kOptimize, self.flags['CXXFLAGS'])
  
  def process_debug(self, mode):
    if mode == 'on':
      self.flags['CXXFLAGS'].append('-g')


class BuildConfiguration(object):
  
  def __init__(self, vars):
    self.libs = { }
    self.progs = { }
    self.flags = { }
    self.vars = vars
  
  def load_file(self, relative_root, build_config):
    full_root = dirname(build_config)
    env = IncludeEnvironment(self, relative_root, full_root)
    exec_env = env.get_exec_environment()
    exec_env.update(**self.vars)
    exec open(build_config).read() in exec_env
  
  def get_library(self, name):
    return self.libs[name]
  
  def get_libraries(self):
    return self.libs
  
  def get_programs(self):
    return self.progs


kVarPattern = re.compile(r'\$(\w+)')
class IncludeEnvironment(object):

  def __init__(self, config, relative_root, full_root):
    self.config = config
    self.relative_root = relative_root
    self.full_root = full_root

  def include(self, name):
    name = kVarPattern.sub(r'%(\1)s', name) % self.config.vars
    relative_root = join(self.relative_root, dirname(name))
    build_config = join(self.full_root, name)
    self.config.load_file(relative_root, build_config)
  
  def library(self, **args):
    lib = Library(self.config, self.relative_root, args)
    self.config.libs[lib.get_name()] = lib
  
  def program(self, **args):
    prog = Program(self.config, self.relative_root, args)
    self.config.progs[prog.get_name()] = prog
  
  def define(self, name, value):
    self.config.vars[name] = value

  def flags(self, name, **args):
    flags = GccFlags(self.config, args)
    self.config.flags[name] = flags

  def get_exec_environment(self):
    return {
      'include': self.include,
      'library': self.library,
      'program': self.program,
      'define': self.define,
      'flags': self.flags
    }


def guess_os():
  system = platform.system()
  if system == 'Darwin':
    return 'darwin'
  elif system == 'Windows':
    return 'win32'
  else:
    return 'linux'


def read_build_file(root, build_config, options):
  vars = {
    'os': guess_os(),
    'mode': 'debug'
  }
  vars.update(**options)
  config = BuildConfiguration(vars)
  config.load_file(root, build_config)
  return config
