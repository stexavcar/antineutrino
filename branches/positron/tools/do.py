#!/usr/bin/env python

import logging
import optparse
from os.path import abspath, join
import sys
import time
import build


_ROOT = abspath(join(sys.argv[0], '..', '..'))
_HERE = abspath('.')


OPTIONS = [
  ('mode', {
    'values': ['debug', 'release'],
    'default': 'release',
    'always_in_variant': True
  }),
  ('wordsize', {
    'values': ['32', '64'],
    'default': '32'
  }),
  ('toolchain', {
    'values': ['msvc', 'gcc', 'guess'],
    'default': 'guess'
  })
]


# --- B u i l d ---


def do_build(options, args):
  command = ['scons', '-Y', _ROOT] + args
  for (key, option) in OPTIONS:
    value = eval('options.%s' % key)
    if value != option['default']:
      command.append('%s=%s' % (key, value))
  output = build.execute_no_capture(command)
  return output.exit_code


# --- T e s t ---


class TestOutput(object):

  def __init__(self, test, command, output):
    self.test = test
    self.command = command
    self.output = output

  def has_failed(self):
    return self.output.exit_code != 0


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

class NunitTestCase(object):

  def __init__(self, name, executable):
    self.name = name
    self.executable = executable
  
  def label(self):
    return self.name
  
  def run(self):
    command = [self.executable, self.name]
    output = build.execute(command)
    return TestOutput(self, command, output)


def do_test(options, args):
  build_exit_code = do_build(options, ['nunit'])
  if build_exit_code != 0:
    return build_exit_code
  nunit = join(_HERE, 'nunit')
  test_list_output = build.execute([nunit, '--list'])
  if test_list_output.exit_code != 0:
    return test_list_output.exit_code
  tests = test_list_output.stdout.strip().split()
  test_cases = [ ]
  for test in tests:
    test_cases.append(NunitTestCase(test, nunit))
  runner = MonoProgressIndicator(test_cases)
  runner.run()


_ACTIONS = {
  'build': do_build,
  'test': do_test
}


def main():
  parser = optparse.OptionParser()
  parser.usage = '%%prog (%s) [options]' % '|'.join(sorted(_ACTIONS.keys()))
  for (key, option) in OPTIONS:
    parser.add_option('--%s' % key, default=option['default'], choices=option['values'])
  parser.add_option('--verbose', action='store_true')
  (options, args) = parser.parse_args()
  if options.verbose:
    logging.basicConfig(level=logging.INFO)
  if (len(args) == 0) or (args[0] not in _ACTIONS.keys()):
    parser.print_usage()
    return 1
  return _ACTIONS[args[0]](options, args[1:])


if __name__ == '__main__':
  sys.exit(main())
