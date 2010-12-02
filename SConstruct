import fnmatch
import os
import platform
from os.path import join

bin_path = 'bin'
src_path = 'src'
pib_path = join(bin_path, 'pib')
obj_path = join(bin_path, 'obj')
class_path = join(bin_path, 'classes')
java_command = 'java -ea'


class ElfFormat(object):

  def summary_command(self):
    return "readelf -h -S $SOURCE"

  def link_command(self):
    return "ld -m elf_i386 $SOURCE /usr/lib/crt?.o -lc -o $TARGET"

  def id(self):
    return "elf"


class MachOFormat(object):

  def summary_command(self):
    return "otool -l -h $SOURCE"

  def link_command(self):
    return "ld $SOURCE /usr/lib/crt?.o -lc -o $TARGET"

  def id(self):
    return "mach-o"


class Platform(object):

  def __init__(self, object_format, architecture):
    self.object_format = object_format
    self.architecture = architecture

  def __str__(self):
    return "Platform { %s, %s }" % (self.object_format, self.architecture)

  @staticmethod
  def get_object_format():
    system = platform.system()
    if system == 'Linux':
      return ElfFormat()
    elif system == 'Darwin':
      return MachOFormat()
    else:
      print system
      assert False

  @staticmethod
  def get_architecture():
    machine = platform.machine()
    if machine in ['x86_64', 'i386', 'i686']:
      return 'ia386'
    else:
      print machine
      assert False

  def get_object_extension(self):
    return ".%s.%s.o" % (self.architecture, self.object_format.id())
  
  def get_executable_extension(self):
    return ".%s.%s" % (self.architecture, self.object_format.id())

  @staticmethod
  def get():
    return Platform(Platform.get_object_format(), Platform.get_architecture())

PLATFORM = Platform.get()

PIB_EXT='.pib'

# Add javatrino as a source since it is required to build
def JavatrinoAdder(source, target, env):
  return target, [javatrino] + source

def CompileGenerator(source, target, env, for_signature):
  flags = ''
  modules = env.get('MODULES')
  if modules:
    for module in modules:
      flags += ' --module %s' % module
  return '%(java)s -cp %(jar)s org.neutrino.main.Compile --root-path %(root)s --outfile $TARGET %(flags)s' % {
    'java': java_command,
    'jar': source[0],
    'root': source[1],
    'flags': flags
  }

PIB_BUILDER = Builder(
  generator = CompileGenerator,
  suffix = PIB_EXT,
  emitter = JavatrinoAdder
)

COMPILE_COMMAND="\
%(java)s -cp %(jar)s org.neutrino.main.Run \
  --pib-path %(pib)s \
  --entry-point compile \
  %(format)s %(arch)s \
  %(input)s $TARGET"
def NeuCompileGenerator(source, target, env, for_signature):
  return COMPILE_COMMAND % {
    'java': java_command,
    'jar': source[0],
    'format': PLATFORM.object_format.id(),
    'arch': PLATFORM.architecture,
    'pib': source[1],
    'input': source[2]
  }

NEU_OBJECT_BUILDER = Builder(
  generator = NeuCompileGenerator,
  suffix = PLATFORM.get_object_extension(),
  emitter = JavatrinoAdder
)

def TestGenerator(source, target, env, for_signature):
  return '%(java)s -cp %(jar)s org.neutrino.main.Run --pib-path %(source)s --entry-point test' % {
    'java': java_command,
    'jar': source[0],
    'source': source[1]
  }

TEST_BUILDER = Builder(
  generator = TestGenerator,
  emitter = JavatrinoAdder
)

env = Environment(
  JARCHDIR=class_path,
  BUILDERS = {
    'Pib': PIB_BUILDER,
    'NeuObject': NEU_OBJECT_BUILDER,
    'Test': TEST_BUILDER
  }
)

javatrino_classes = env.Java(class_path, src_path)
javatrino = env.Jar(join(bin_path, 'javatrino'), class_path)

def BuildPib(target, source, **kwargs):
  result = env.Pib(target, Dir(source), **kwargs)
  for base, dirs, files in os.walk(source):
    goodfiles = fnmatch.filter(files, '*.n')
    for f in goodfiles:
      env.Depends(result, join(base, f))
  return result

libpib = BuildPib(join(pib_path, 'lib'), 'lib')

modules = ['org::neutrino::neuneu', 'org::neutrino::neuneu::test', 'org::neutrino::neuneu::value']
neuneupib = BuildPib(join(pib_path, 'neuneu'), 'lib', MODULES=modules)

neuneuobj = env.NeuObject(join(obj_path, 'neuneu'), [libpib, neuneupib])

libtest = env.Test('libtest', libpib)

env.Command('neuneu-summary', neuneuobj, PLATFORM.object_format.summary_command())

# Building an executable
exe_name = 'neuneu%s' % PLATFORM.get_executable_extension()
exe_path = join(obj_path, exe_name)
exe = env.Command(exe_path, neuneuobj, PLATFORM.object_format.link_command())
env.Alias('neuneu-exe', exe)

# Running neuneu
run_exe = env.Command("neuneu", [exe], "./%s" % exe_path)

env.Alias('all', [libtest, neuneuobj, run_exe])

Default('all')
