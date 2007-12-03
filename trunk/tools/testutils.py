from os.path import abspath, basename, join, commonprefix
import subprocess, os, sys

def execute(executable, args):
  process = subprocess.Popen(
    args = args,
    executable = abspath(str(executable)),
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
  if selected == base[:base.index('.')]: return True
  return False

class CCTestCase:
  def __init__(self, executable, test_case):
    self.test_case = test_case
    self.executable = executable
  def __str__(self):
    return self.test_case + " (" + basename(str(self.executable)) + ")"
  def run(self):
    (exit_code, output, errors) = execute(self.executable, [ '', self.test_case ])
    status = None
    if exit_code == 0: return ('passed', output, errors)
    else: return ('failed', output, errors)
  def command(self):
    list = [ self.executable, self.test_case ]
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
    (exit_code, output, errors) = execute(self.executable, [ '', self.test_case ])
    if exit_code == 0: return ('passed', output, errors)
    else: return ('failed', output, errors)

def truncate(str, length):
  if len(str) > (length - 3): return str[:length] + "..."
  else: return str

status_line = "[\033[32mP%(passed) 4d\033[0m|\033[31mF%(failed) 4d\033[0m|\033[34mR%(remaining) 4d\033[0m]: %(test)s"

def escape_string(s):
  s = s.replace('\\', '\\\\')
  return s

class ProgressIndicator:
  def __init__(self, count):
    self.remaining = count
    self.passed = 0
    self.failed = 0

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
    print status_line % {
      'passed':    self.passed,
      'remaining': self.remaining,
      'failed':    self.failed,
      'test':      name
    },
  def test_failed(self, test):
    self.clear_line()
    print "--- Failed: " + str(test) + " ---"
    print "Command: " + test.command()
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

def run_neutrino_tests(all_tests, output):
  if output == 'color':
    progress = ColorProgressIndicator(len(all_tests))
  elif output == 'dots':
    progress = DotsProgressIndicator(len(all_tests))
  else:
    assert False
  for test in all_tests:
    progress.start_test(test)
    (status, stdout, stderr) = test.run()
    stdout = stdout.strip()
    stderr = stderr.strip()
    if status == 'failed':
      progress.test_failed(test)
    elif len(stdout) > 0 or len(stderr) > 0:
      progress.test_had_output(test)
    if status == 'passed':
      progress.test_passed(test)
    progress.show_output(stdout, stderr)
  test = "Done"
  progress.tests_done()
  print
