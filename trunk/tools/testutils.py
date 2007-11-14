from os.path import abspath, basename, join
import subprocess
import os

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
    (exit_code, output, errors) = execute(self.executable, [ '', self.test_case ])
    if exit_code == 0: return ('passed', output, errors)
    else: return ('failed', output, errors)

def truncate(str, length):
  if len(str) > (length - 3): return str[:length] + "..."
  else: return str

status_line = "[\033[32mP%(passed) 3d\033[0m|\033[31mF%(failed) 3d\033[0m|\033[34mR%(remaining) 3d\033[0m]: %(test)s"

def run_neutrino_tests(all_tests):
  remaining = len(all_tests)
  passed = 0
  failed = 0
  def clear_line():
    print "\033[1K\r",
  def print_progress():
    print status_line % {
      'passed':    passed,
      'remaining': remaining,
      'failed': failed,
      'test': truncate(str(test), 40)
    }, 
  for test in all_tests:
    print_progress();
    (status, stdout, stderr) = test.run()
    stdout = stdout.strip()
    stderr = stderr.strip()
    if status == 'failed':
      clear_line()
      print "--- Failed: " + str(test) + " ---"
      print "Command: " + test.command()
    elif len(stdout) > 0 or len(stderr) > 0:
      clear_line()
      print "--- " + str(test) + " ---"
    if len(stderr) > 0:
      print "\033[31m" + stderr + "\033[0m"
    if len(stdout) > 0:
      print "\033[1m" + stdout + "\033[0m"
    if status == 'failed':
      failed = failed + 1
    else:
      passed = passed + 1
    remaining = remaining - 1
    clear_line()
  test = "Done"
  print_progress()
  print
