import logging
from os.path import dirname, join
import platform
import re
import subprocess


class CommandOutput(object):

  def __init__(self, exit_code, stdout, stderr):
    self.exit_code = exit_code
    self.stdout = stdout
    self.stderr = stderr
  
  def __str__(self):
    return 'command output { code: %i, stdout: "%s", stderr: "%s" }' % (self.exit_code, self.stdout, self.stderr)


def execute(args):
  logging.info(' '.join(args))
  process = subprocess.Popen(
    args = args,
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE,
    shell = is_win32()
  )
  exit_code = process.wait()
  output = process.stdout.read()
  errors = process.stderr.read()
  return CommandOutput(exit_code, output, errors)


def execute_no_capture(args):
  logging.info(' '.join(args))
  process = subprocess.Popen(
    args = args,
    shell = is_win32()
  )
  exit_code = process.wait()
  return CommandOutput(exit_code, "", "")


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
        result += self.config.vars.get(entry[1:], [])
      else:
        result.append(entry)
    return result
  
  def get_domain(self):
    return self.args.get('domain', None)
  
  def get_libraries(self):
    srcs = self.get_dependencies()
    return [p[1:] for p in srcs if p.startswith(':')]
  
  def get_system_libraries(self):
    srcs = self.get_dependencies()
    return [p[1:] for p in srcs if p.startswith('@')]
  
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
      for (key, value) in self.config.flags[parent].to_dict().items():
        self.flags[key] = list(value)
    else:
      self.flags = {
        'CXXFLAGS': [ '$WARNINGFLAGS' ],
        'WARNINGFLAGS': [ ],
        'CPPDEFINES': [ ],
        'LINKFLAGS': [ ],
        'CPPPATH': [ ]
      }
    self.process_warnings(self.args.get('warnings', []))
    self.process_dialects(self.args.get('dialect', []))
    self.process_codegen(self.args.get('codegen', []))
    self.process_defines(self.args.get('defines', []))
    self.process_includepath(self.args.get('includepath', []))
    self.process_optimize(self.args.get('optimize', None))
    self.process_debug(self.args.get('debug', []))
    self.process_wordsize(self.args.get('wordsize', None))
    self.process_link(self.args.get('link', []))
    return self.flags
    
  def process_includepath(self, includepath):
    self.flags['CPPPATH'] += includepath
    
  def process_defines(self, defines):
    self.flags['CPPDEFINES'] += defines


def get_flag_processor(config, args):
  if config.vars.get('toolchain', None) == 'msvc':
    return MsvcFlags(config, args)
  else:
    return GccFlags(config, args)


class MsvcFlags(Flags):

  def __init__(self, config, args):
    super(MsvcFlags, self).__init__(config, args)
  
  kDialect = {
    'nologo': '/nologo'
  }
  
  kWarning = {
    'no-initializer-this': '/wd4355'
  }

  kCodegen = {
    'no-rtti': '/GR-',
    'no-exceptions': '/EHs-c-',
    'static-multithread-debug': '/MTd'
  }

  kOptimize = {
    'speed': '/O2',
    'no': '/Od'
  }
  
  kDebug = {
    'store-info': '/Zi',
    'runtime-error-checking': '/RTC1'
  }

  kLink = {
    'debug-info': '/debug'
  }

  kWordsize = { }
  
  def process_dialects(self, dialects):
    self.apply_flag_map(dialects, MsvcFlags.kDialect, self.flags['CXXFLAGS'])
  
  def process_warnings(self, warnings):
    self.apply_flag_map(warnings, MsvcFlags.kWarning, self.flags['WARNINGFLAGS'])
  
  def process_codegen(self, codegen):
    self.apply_flag_map(codegen, MsvcFlags.kCodegen, self.flags['CXXFLAGS'])
    
  def process_optimize(self, level):
    self.apply_flag_map(level, MsvcFlags.kOptimize, self.flags['CXXFLAGS'])

  def process_wordsize(self, size):
    self.apply_flag_map(size, MsvcFlags.kWordsize, self.flags['CXXFLAGS'])
    self.apply_flag_map(size, MsvcFlags.kWordsize, self.flags['LINKFLAGS'])
  
  def process_debug(self, mode):
    self.apply_flag_map(mode, MsvcFlags.kDebug, self.flags['CXXFLAGS'])

  def process_link(self, flags):
    self.apply_flag_map(flags, MsvcFlags.kLink, self.flags['LINKFLAGS'])

class GccFlags(Flags):

  kDialect = {
    'ansi': '-ansi',
    'pedantic': '-pedantic',
  }
  
  kCodegen = {
    'no-rtti': '-fno-rtti',
    'no-exceptions': '-fno-exceptions'
  }
  
  kWarning = {
    'all': '-Wall',
    'extra': '-Wextra',
    'no-unused-parameter': '-Wno-unused-parameter',
    'no-non-virtual-dtor': '-Wno-non-virtual-dtor',
    'shadow': '-Wshadow'
  }
  
  kOptimize = {
    'no': '-O0',
    'speed': '-O3'
  }
  
  kWordsize = {
    '32': '-m32',
    '64': '-m64'
  }
  
  kDebug = {
    'store-info': '-g'
  }
  
  kLink = {
  
  }

  def __init__(self, config, args):
    super(GccFlags, self).__init__(config, args)

  def process_dialects(self, dialects):
    self.apply_flag_map(dialects, GccFlags.kDialect, self.flags['CXXFLAGS'])
  
  def process_warnings(self, warnings):
    self.apply_flag_map(warnings, GccFlags.kWarning, self.flags['WARNINGFLAGS'])
  
  def process_codegen(self, codegen):
    self.apply_flag_map(codegen, GccFlags.kCodegen, self.flags['CXXFLAGS'])
  
  def process_optimize(self, level):
    self.apply_flag_map(level, GccFlags.kOptimize, self.flags['CXXFLAGS'])

  def process_wordsize(self, size):
    self.apply_flag_map(size, GccFlags.kWordsize, self.flags['CXXFLAGS'])
    self.apply_flag_map(size, GccFlags.kWordsize, self.flags['LINKFLAGS'])
  
  def process_debug(self, mode):
    self.apply_flag_map(mode, GccFlags.kDebug, self.flags['CXXFLAGS'])

  def process_link(self, flags):
    self.apply_flag_map(flags, GccFlags.kLink, self.flags['LINKFLAGS'])


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
    flags = get_flag_processor(self.config, args)
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


def is_win32():
  return guess_os() == 'win32'


def read_build_file(root, build_config, options):
  vars = {
    'os': guess_os(),
    'mode': 'debug'
  }
  vars.update(**options)
  config = BuildConfiguration(vars)
  config.load_file(root, build_config)
  return config
