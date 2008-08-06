from os.path import join, exists
import test


class CcTestCase(test.TestCase):

  def __init__(self, config, mode, name):
    self.name = name
    self.mode = mode
    self.config = config

  def commands(self):
    cctests = join(self.config.context.buildspace, self.mode, 'cctests')
    return [ [cctests, self.name] ]


def weak_match(path, name):
  return name in path.split(':')


def strong_match(path, file, name):
  [ pfile, pname ] = path.split(':')
  return file == pfile and name == pname


class CcTestConfiguration(test.TestConfiguration):

  def __init__(self, context, root):
    super(CcTestConfiguration, self).__init__(context, root)
    self.has_test_list = False
    self.test_list = None

  def get_build_requirements(self):
    return ['tests']

  def get_test_list(self, mode):
    if not self.has_test_list:
      self.has_test_list = True
      cctests = join(self.context.buildspace, mode, 'cctests')
      output = test.execute([cctests, '--list'])
      if output.exit_code != 0:
        print output.stdout
        print output.stderr
        return [ ]
      self.test_list = output.stdout.strip().split("\n")
    return self.test_list

  def list_tests(self, path, mode):
    if len(path) > 2: return [ ]
    test_list = self.get_test_list(mode)
    if len(path) == 0:
      return [ CcTestCase(self, mode, t) for t in test_list ]
    elif len(path) == 1:
      return [ CcTestCase(self, mode, t) for t in test_list if weak_match(t, path[0]) ]
    else:
      return [ CcTestCase(self, mode, t) for t in test_list if strong_match(t, path[0], path[1]) ]

def get_configuration(context, root):
  return CcTestConfiguration(context, root)
