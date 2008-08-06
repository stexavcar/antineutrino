#!/usr/bin/python

import imp
import optparse
import os
from os.path import join, dirname, abspath, basename
import platform
import subprocess
import sys
import time
import utils


VERBOSE = False


class CommandOutput(object):

  def __init__(self, exit_code, stdout, stderr):
    self.exit_code = exit_code
    self.stdout = stdout
    self.stderr = stderr


class TestCase(object):

  def run(self):
    commands = self.commands()
    for command in commands:
      output = execute(command)
      if output.exit_code != 0:
        break
    return TestOutput(self, command, output)


class TestOutput(object):

  def __init__(self, test, command, output):
    self.test = test
    self.command = command
    self.output = output

  def has_failed(self):
    return self.output.exit_code != 0


def execute(args):
  if VERBOSE: print " ".join(args)
  process = subprocess.Popen(
    args = args,
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE
  )
  exit_code = process.wait()
  output = process.stdout.read()
  errors = process.stderr.read()
  return CommandOutput(exit_code, output, errors)


def execute_no_capture(args):
  if VERBOSE: print " ".join(args)
  process = subprocess.Popen(args = args)
  exit_code = process.wait()
  return CommandOutput(exit_code, "", "")


def carcdr(path):
  if len(path) == 0:
    return (None, [ ])
  else:
    return (path[0], path[1:])


class TestConfiguration(object):

  def __init__(self, context, root):
    self.context = context
    self.root = root

  def get_build_requirements(self):
    return [ ]

  def list_tests(self, path, mode):
    return [ ]


class PyneuTestCase(TestCase):

  def __init__(self, config, mode, name):
    self.name = name
    self.config = config
    self.mode = mode

  def full_name(self, name):
    system = platform.system()
    if system == 'Linux':
      return 'lib%s.so' % name
    elif system == 'Windows':
      return '%s.dll' % name
    else:
      return name

  def get_lib_list(self, raw_libs):
    return [ join(self.mode, self.full_name(l)) for l in raw_libs ]    

  def commands(self):
    test = join(self.config.root, self.name + ".n")
    libs = join(self.config.context.workspace, 'lib')
    compiler = join(self.config.context.workspace, 'tools', 'pyneu', 'main.py')
    consts = join(self.config.context.workspace, 'src', 'utils', 'consts.h')
    runtime = join(self.config.context.buildspace, self.mode, 'neutrino')
    dylibs = self.get_lib_list(['natives'])
    return [
      ['python', compiler, '-c', consts, '-o', 'temp.nc', test, libs],
      [runtime, '--images[', 'temp.nc', ']', '--libs['] + dylibs + [']']
    ]


class PyneuTestConfiguration(TestConfiguration):

  def __init__(self, context, root):
    super(PyneuTestConfiguration, self).__init__(context, root)

  def get_build_requirements(self):
    return [ 'program' ]

  def list_tests(self, path, mode):
    if len(path) > 1: return [ ]
    cases = [ c[:-2] for c in os.listdir(self.root) if c.endswith('.n') ]
    if len(path) == 0:
      return [ PyneuTestCase(self, mode, t) for t in cases ]
    else:
      return [ PyneuTestCase(self, mode, t) for t in cases if t == path[0] ]


class TestSuite(object):

  def __init__(self, name):
    self.name_ = name

  def name(self):
    return self.name_


class TestRepository(TestSuite):

  def __init__(self, path):
    super(TestRepository, self).__init__(basename(path))
    self.path = path
    self.is_loaded = False
    self.config = None

  def get_configuration(self, context):
    if self.is_loaded:
      return self.config
    self.is_loaded = True
    file = None
    try:
      (file, pathname, description) = imp.find_module('setup', [ self.path ])
      module = imp.load_module(self.path, file, pathname, description)
      self.config = module.get_configuration(context, self.path)
    finally:
      if file:
        file.close()
    return self.config

  def get_build_requirements(self, path, context):
    return self.get_configuration(context).get_build_requirements()

  def list_tests(self, path, context, mode):
    return self.get_configuration(context).list_tests(path, mode)


class LiteralTestSuite(TestSuite):

  def __init__(self, tests):
    super(LiteralTestSuite, self).__init__('root')
    self.tests = tests

  def get_build_requirements(self, path, context):
    (name, rest) = carcdr(path)
    result = [ ]
    for test in self.tests:
      if not name or name == test.name():
        result += test.get_build_requirements(rest, context)
    return result

  def list_tests(self, path, context, mode):
    (name, rest) = carcdr(path)
    result = [ ]
    for test in self.tests:
      if not name or name == test.name():
        result += test.list_tests(rest, context, mode)
    return result


class Context(object):

  def __init__(self, workspace, buildspace):
    self.workspace = workspace
    self.buildspace = buildspace


class ProgressIndicator(object):

  def __init__(self, cases):
    self.cases = cases
    self.succeeded = 0
    self.failed = 0
    self.remaining = len(self.cases)
    self.total = len(self.cases)
    self.failed_tests = [ ]

  def run(self):
    self.starting()
    for case in self.cases:
      self.about_to_run(case)
      output = case.run()
      if output.has_failed():
        self.failed += 1
        self.failed_tests.append(output)
      else:
        self.succeeded += 1
      self.remaining -= 1
      self.has_run(output)
    self.done()


class MonochromeProgressIndicator(ProgressIndicator):

  def starting(self):
    print 'Running %i tests' % len(self.cases)

  def done(self):
    print
    for failed in self.failed_tests:
      print "=== %s ===" % failed.test.name
      print "Command: %s" % " ".join(failed.command)
      if failed.output.stderr:
        print "--- stderr ---"
        print failed.output.stderr.strip()
      if failed.output.stdout:
        print "--- stdout ---"
        print failed.output.stdout.strip()
    if len(self.failed_tests) == 0:
      print "==="
      print "=== All tests succeeded"
      print "==="
    else:
      print
      print "==="
      print "=== %i tests failed" % len(self.failed_tests)
      print "==="


class VerboseProgressIndicator(MonochromeProgressIndicator):

  def about_to_run(self, case):
    print '%s:' % case.name,
    sys.stdout.flush()

  def has_run(self, output):
    if output.has_failed():
      print "FAIL"
    else:
      print "pass"


class DotsProgressIndicator(MonochromeProgressIndicator):

  def about_to_run(self, case):
    pass

  def has_run(self, output):
    if output.has_failed():
      sys.stdout.write('F')
      sys.stdout.flush()
    else:
      sys.stdout.write('.')
      sys.stdout.flush()


class CompactProgressIndicator(ProgressIndicator):

  def __init__(self, cases, templates):
    super(CompactProgressIndicator, self).__init__(cases)
    self.templates = templates
    self.last_status_length = 0
    self.start_time = time.time()
  
  def starting(self):
    pass
  
  def done(self):
    self.print_progress('Done')
  
  def about_to_run(self, case):
    self.print_progress(case.name)
  
  def has_run(self, output):
  	if output.has_failed():
  	  print "--- Failed: %s ---" % str(output.test.name)
  	  print "Command: %s" % " ".join(output.command)
  	stdout = output.output.stdout.strip()
  	if len(stdout):
  	  print self.templates['stdout'] % stdout
  	stderr = output.output.stderr.strip()
  	if len(stderr):
  	  print self.templates['stderr'] % stderr

  def truncate(self, str, length):
    if length and (len(str) > (length - 3)):
      return str[:(length-3)] + "..."
    else:
      return str

  def print_progress(self, name):
    self.clear_line(self.last_status_length)
    elapsed = time.time() - self.start_time
    status = self.templates['status_line'] % {
      'passed': self.succeeded,
      'remaining': (((self.total - self.remaining) * 100) // self.total),
      'failed': self.failed,
      'test': name,
      'mins': int(elapsed) / 60,
      'secs': int(elapsed) % 60
    }
    status = self.truncate(status, 78)
    self.last_status_length = len(status)
    print status,
    sys.stdout.flush()


class ColorProgressIndicator(CompactProgressIndicator):

  def __init__(self, cases):
    templates = {
      'status_line': "[%(mins)02i:%(secs)02i|\033[34m%%%(remaining) 4d\033[0m|\033[32m+%(passed) 4d\033[0m|\033[31m-%(failed) 4d\033[0m]: %(test)s",
      'stdout': "\033[1m%s\033[0m",
      'stderr': "\033[31m%s\033[0m",
    }
    super(ColorProgressIndicator, self).__init__(cases, templates)
  
  def clear_line(self, last_line_length):
    print "\033[1K\r",


class MonoProgressIndicator(CompactProgressIndicator):

  def __init__(self, cases):
    templates = {
      'status_line': "[%(mins)02i:%(secs)02i|%%%(remaining) 4d|+%(passed) 4d|-%(failed) 4d]: %(test)s",
      'stdout': '%s',
      'stderr': '%s',
      'clear': lambda last_line_length: ("\r" + (" " * last_line_length) + "\r"),
      'max_length': 78
    }
    super(MonoProgressIndicator, self).__init__(cases, templates)
  
  def clear_line(self, last_line_length):
    print ("\r" + (" " * last_line_length) + "\r"),


def run_test_cases(cases):
  progress = ColorProgressIndicator(cases)
  progress.run()


def build_options():
  result = optparse.OptionParser()
  result.add_option("-m", "--mode", help="The test mode in which to run")
  result.add_option("-v", "--verbose", help="Verbose output", default=False, action="store_true")
  return result


def validate_options(options):
  mode = options.mode
  if mode == 'all': mode = ['paranoid', 'debug', 'product']
  elif not mode: mode = ['paranoid']
  else: mode = [mode]
  options.mode = mode
  return True


def build_requirements(workspace, requirements, mode):
  command_line = ['scons', '-Y', workspace, 'mode=' + ",".join(mode)] + requirements
  output = execute_no_capture(command_line)
  return output.exit_code == 0
  

def main():
  parser = build_options()
  (options, args) = parser.parse_args()
  if not validate_options(options):
    parser.print_help()
    return 1
  global VERBOSE
  VERBOSE = options.verbose
  workspace = abspath(join(dirname(sys.argv[0]), '..'))
  tests = utils.read_lines_from(join(workspace, 'test', 'suites.list'))
  repositories = [TestRepository(join(workspace, 'test', name)) for name in tests]
  root = LiteralTestSuite(repositories)
  if len(args) == 0:
    paths = [[]]
  else:
    paths = [ ]
    for arg in args:
      path = [ arg for arg in arg.split('/') if len(arg) > 0 ]
      paths.append(path)

  # First build the required targets
  reqs = [ ]
  context = Context(workspace, abspath('.'))
  for path in paths:
    reqs += root.get_build_requirements(path, context)
  reqs = list(set(reqs))
  if len(reqs) > 0:
    if not build_requirements(workspace, reqs, options.mode):
      return 1

  # Then list the tests
  cases = [ ]
  for path in paths:
    for mode in options.mode:
      cases += root.list_tests(path, context, mode)
  
  if len(cases) == 0:
    print "No tests to run."
  else:
    run_test_cases(cases)

  return 0


if __name__ == '__main__':
  sys.exit(main())
