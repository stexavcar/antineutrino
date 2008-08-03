#!/usr/bin/python

import imp
import optparse
import os
from os.path import join, dirname, abspath, basename
import subprocess
import sys
import utils


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
  print " ".join(args)
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

  def list_tests(self, path):
    return [ ]


class PyneuTestCase(TestCase):

  def __init__(self, config, name):
    self.name = name
    self.config = config

  def commands(self):
    test = join(self.config.root, self.name + ".n")
    libs = join(self.config.context.workspace, 'lib')
    compiler = join(self.config.context.workspace, 'tools', 'pyneu', 'main.py')
    consts = join(self.config.context.workspace, 'src', 'utils', 'consts.h')
    runtime = join(self.config.context.buildspace, 'bin', 'neutrino')
    return [
      ['python', compiler, '-c', consts, '-o', 'temp.nc', test, libs],
      [runtime, '--images[', 'temp.nc', ']', '--libs[', ']']
    ]


class PyneuTestConfiguration(TestConfiguration):

  def __init__(self, context, root):
    super(PyneuTestConfiguration, self).__init__(context, root)

  def get_build_requirements(self):
    return [ 'program' ]

  def list_tests(self, path):
    if len(path) > 1: return [ ]
    cases = [ c[:-2] for c in os.listdir(self.root) if c.endswith('.n') ]
    if len(path) == 0:
      return [ PyneuTestCase(self, t) for t in cases ]
    else:
      return [ PyneuTestCase(self, t) for t in cases if t == path[0] ]


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

  def list_tests(self, path, context):
    return self.get_configuration(context).list_tests(path)


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

  def list_tests(self, path, context):
    (name, rest) = carcdr(path)
    result = [ ]
    for test in self.tests:
      if not name or name == test.name():
        result += test.list_tests(rest, context)
    return result


class Context(object):

  def __init__(self, workspace, buildspace):
    self.workspace = workspace
    self.buildspace = buildspace


def build_options():
  result = optparse.OptionParser()
  return result


def validate_options(options):
  return True


def build_requirements(workspace, requirements):
  command_line = ['scons', '-Y', workspace] + requirements
  output = execute_no_capture(command_line)
  return output.exit_code == 0


class ProgressIndicator(object):

  def __init__(self, cases):
    self.cases = cases
    self.succeeded = 0
    self.failed = 0
    self.remaining = len(self.cases)
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



class CompactProgressIndicator(MonochromeProgressIndicator):

  def about_to_run(self, case):
    pass

  def has_run(self, output):
    if output.has_failed():
      sys.stdout.write('F')
      sys.stdout.flush()
    else:
      sys.stdout.write('.')
      sys.stdout.flush()


def run_test_cases(cases):
  progress = VerboseProgressIndicator(cases)
  progress.run()


def main():
  parser = build_options()
  (options, args) = parser.parse_args()
  if not validate_options(options):
    parser.print_help()
    return 1
  workspace = abspath(join(dirname(sys.argv[0]), '..'))
  tests = utils.read_lines_from(join(workspace, 'test', 'suites.list'))
  repositories = [TestRepository(join(workspace, 'test', name)) for name in tests]
  root = LiteralTestSuite(repositories)
  paths = [ ]
  for arg in args:
    path = [ arg for arg in arg.split('/') if len(arg) > 0 ]
    paths.append(path)

  # First build the required targets
  reqs = [ ]
  context = Context(workspace, abspath('.'))
  for path in paths:
    reqs += root.get_build_requirements(path, context)
  if len(reqs) > 0 and not build_requirements(workspace, reqs):
    return 1

  # Then list the tests
  cases = [ ]
  for path in paths:
    cases += root.list_tests(path, context)
  run_test_cases(cases)

  return 0


if __name__ == '__main__':
  sys.exit(main())
