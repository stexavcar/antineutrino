from os.path import abspath, basename
import os

class CCTestCase:
  def __init__(self, executable, test_case):
    self.test_case = test_case
    self.executable = executable
  def __str__(self):
    return self.test_case + " (" + basename(str(self.executable)) + ")"
  def run(self):
    command = self.command()
    from subprocess import Popen
    exit_code = os.spawnv(os.P_WAIT, abspath(str(self.executable)), [self.test_case] )
    os.spawnv(os.P_WAIT, "sleep 1", [])
    status = None
    if exit_code == 0: status = 'passed'
    else: status = 'failed'
    return [ status ]
  def command(self):
    list = [ str(self.executable), self.test_case ]
    return ' '.join(list)

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
