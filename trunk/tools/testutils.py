from os.path import abspath, basename, join
import os

def execute(executable, args):
  from subprocess import Popen
  exit_code = os.spawnv(os.P_WAIT, abspath(str(executable)), args)
  return exit_code

class CCTestCase:
  def __init__(self, executable, test_case):
    self.test_case = test_case
    self.executable = executable
  def __str__(self):
    return self.test_case + " (" + basename(str(self.executable)) + ")"
  def run(self):
    exit_code = execute(self.executable, [ '', self.test_case ])
    status = None
    if exit_code == 0: return [ 'passed' ]
    else: return [ 'failed' ]
  def command(self):
    list = [ str(self.executable), self.test_case ]
    return ' '.join(list)

class PyNeutrinoTestCase:
  def __init__(self, executable, test_case):
    self.executable = executable
    self.test_case = test_case
  def __str__(self):
    return basename(self.test_case)[:-3]
  def command(self):
    list = [ self.executable, self.test_case ]
    return ' '.join(list)
  def run(self):
    exit_code = execute(self.executable, [ '', self.test_case ])
    if exit_code == 0: return [ 'passed' ]
    else: return [ 'failed' ]

def truncate(str, length):
  if len(str) > (length - 3): return str[:length] + "..."
  else: return str

status_line = "[\033[32mP%(passed) 3d\033[0m|\033[33mF%(failed) 3d\033[0m|\033[31mE%(errors) 3d\033[0m|\033[34mR%(remaining) 3d\033[0m]: %(test)s"

def run_neutrino_tests(all_tests):
  remaining = len(all_tests)
  passed = 0
  failed = 0
  errors = 0
  def print_progress():
    print status_line % {
      'passed':    passed,
      'remaining': remaining,
      'failed': failed,
      'errors': errors,
      'test': truncate(str(test), 40)
    }, 
  for test in all_tests:
    print_progress();
    [status] = test.run()
    test_failed = False
    if status == 'failed':
      failed = failed + 1
      test_failed = True
    else:
      passed = passed + 1
    remaining = remaining - 1
    print "\033[1K\r",
    if test_failed:
      print "\033[4mFailed:\033[0m " + str(test)
      print "\033[4mCommand:\033[0m " + test.command()
  test = "Done"
  print_progress()
  print
