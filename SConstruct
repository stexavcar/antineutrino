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
      return 'elf'
    elif system == 'Darwin':
      return 'mach-o'
    else:
      assert False

  @staticmethod
  def get_architecture():
    machine = platform.machine()
    if machine == 'x86_64':
      return 'ia386'
    else:
      assert False

  def get_object_extension(self):
    return ".%s-%s" % (self.architecture, self.object_format)

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
  module = env.get('MODULE')
  if module:
    flags += '--module %s' % module
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
    'format': PLATFORM.object_format,
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

neuneupib = BuildPib(join(pib_path, 'neuneu'), 'lib', MODULE='org::neutrino::neuneu')

neuneuobj = env.NeuObject(join(obj_path, 'neuneu'), [libpib, neuneupib])

libtest = env.Test('libtest', libpib)

env.Alias('all', [libtest, neuneuobj])

Default('all')
