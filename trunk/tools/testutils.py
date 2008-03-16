from os.path import abspath, basename, join, commonprefix
import subprocess, os, sys, time, re

def execute(args):
  process = subprocess.Popen(
    args = args,
    stdout = subprocess.PIPE,
    stderr = subprocess.PIPE
  )
  exit_code = process.wait()
  output = process.stdout.read()
  errors = process.stderr.read()
  return (exit_code, output, errors)

def relative(base, str):
  prefix = commonprefix([base, str])
  return str[len(prefix)+1:]

def is_selected(config, test):
  if not config['test']: return True
  selected = config['test']
  base = basename(test)
  if selected == base: return True
  if selected == base[:base.find('.')]: return True
  return False

class Result:
  PASSED = 'passed'
  FAILED = 'failed'
  def __init__(self, status, output="", error=""):
    self.status = status
    self.output = output
    self.error = error

class CCTestCase:
  def __init__(self, executable, test_case, target):
    self.test_case = test_case
    self.executable = executable
    self.target = target
  def __str__(self):
    return self.target + ' ' + self.test_case + " (" + basename(str(self.executable)) + ")"
  def run(self):
    (exit_code, output, errors) = execute([ abspath(self.executable), self.test_case ])
    status = None
    if exit_code == 0: return Result(Result.PASSED, output=output, error=errors)
    else: return Result(Result.FAILED, output=output, error=errors)
  def command(self):
    list = [ self.executable, self.test_case ]
    return ' '.join(list)

class PyNeutrinoTestCase:
  def __init__(self, executable, test_case, libs, target):
    self.executable = executable
    self.test_case = test_case
    self.libs = libs
    self.target = target
  def __str__(self):
    return self.target + ' ' + basename(self.test_case)[:-3]
  def args(self):
    return [ '--images[', self.test_case, ']', '--libs[' ] + self.libs + [ ']' ]
  def command(self):
    list = [ self.executable ] + self.args()
    return ' '.join(list)
  def run(self):
    (exit_code, output, errors) = execute([ abspath(self.executable) ] + self.args())
    if exit_code == 0: return Result(Result.PASSED, output=output, error=errors)
    else: return Result(Result.FAILED, output=output, error=errors)

class CustomTestCase:
  def __init__(self, name, fun):
    self.name = name
    self.fun = fun
  def __str__(self):
    return self.name
  def command(self):
    return None
  def run(self):
    return self.fun();

def truncate(str, length):
  if len(str) > (length - 3): return str[:length] + "..."
  else: return str

status_line = "[%(mins)02i:%(secs)02i|\033[34m%%%(remaining) 4d\033[0m|\033[32m+%(passed) 4d\033[0m|\033[31m-%(failed) 4d\033[0m]: %(test)s"

class ProgressIndicator:
  def __init__(self, count):
    self.remaining = count
    self.total = count
    self.passed = 0
    self.failed = 0
    self.start_time = time.time()

class DotsProgressIndicator(ProgressIndicator):
  def __init__(self, count):
    ProgressIndicator.__init__(self, count)
  def start_test(self, test):
    pass
  def test_failed(self, test):
    sys.stdout.write('F')
    self.failed = self.failed + 1
  def test_passed(self, test):
    sys.stdout.write('.')
    self.passed = self.passed + 1
  def test_had_output(self, test):
    self.test_passed(test)
  def show_output(self, stdout, stderr):
    pass
  def tests_done(self):
    print
    if self.failed > 0:
      print "Ran", self.remaining, "tests, failed", self.failed
    else:
      print "Ran", self.remaining, "tests, all passed"

class ColorProgressIndicator(ProgressIndicator):
  def __init__(self, count):
    ProgressIndicator.__init__(self, count)
  def clear_line(self):
    print "\033[1K\r",
  def start_test(self, test):
    self.print_progress(truncate(str(test), 40))
  def print_progress(self, name):
    self.clear_line()
    elapsed = time.time() - self.start_time
    print status_line % {
      'passed':    self.passed,
      'remaining': (((self.total - self.remaining) * 100) // self.total),
      'failed':    self.failed,
      'test':      name,
      'mins':      int(elapsed) / 60,
      'secs':      int(elapsed) % 60
    },
  def test_failed(self, test):
    self.clear_line()
    print "--- Failed: " + str(test) + " ---"
    command = test.command()
    if command: print "Command: " + command
    self.failed = self.failed + 1
    self.remaining = self.remaining - 1
  def test_passed(self, test):
    self.passed = self.passed + 1
    self.remaining = self.remaining - 1
  def test_had_output(self, test):
    self.clear_line()
    print "--- " + str(test) + " ---"
  def show_output(self, stdout, stderr):
    if len(stdout) > 0:
      print "\033[1m" + stdout + "\033[0m"
    if len(stderr) > 0:
      print "\033[31m" + stderr + "\033[0m"
  def tests_done(self):
    self.print_progress('Done')

def strip_newlines(str):
  return str.rstrip()

def run_neutrino_tests(all_tests, output):
  if output == 'color':
    progress = ColorProgressIndicator(len(all_tests))
  elif output == 'dots':
    progress = DotsProgressIndicator(len(all_tests))
  else:
    assert False
  for test in all_tests:
    progress.start_test(test)
    result = test.run()
    stdout = strip_newlines(result.output)
    stderr = strip_newlines(result.error)
    if result.status == Result.FAILED:
      progress.test_failed(test)
    elif len(stdout) > 0 or len(stderr) > 0:
      progress.test_had_output(test)
    if result.status == Result.PASSED:
      progress.test_passed(test)
    progress.show_output(stdout, stderr)
  test = "Done"
  progress.tests_done()
  print
