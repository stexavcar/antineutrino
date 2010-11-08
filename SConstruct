import fnmatch
import os
from os.path import join

bin_path = 'bin'
src_path = 'src'
pib_path = join(bin_path, 'pib')
obj_path = join(bin_path, 'obj')
class_path = join(bin_path, 'classes')
java_command = 'java -ea'

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
  suffix = '.pib',
  emitter = JavatrinoAdder
)

def NeuCompileGenerator(source, target, env, for_signature):
  return '%(java)s -cp %(jar)s org.neutrino.main.Run --pib-path %(pib)s --entry-point compile %(input)s $TARGET' % {
    'java': java_command,
    'jar': source[0],
    'pib': source[1],
    'input': source[2]
  }

NEU_OBJECT_BUILDER = Builder(
  generator = NeuCompileGenerator,
  suffix = '.no',
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
