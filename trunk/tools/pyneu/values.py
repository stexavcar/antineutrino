
class Value(object):
  pass


class Smi(Value):
  
  def __init__(self, value):
    super(Smi, self).__init__()
    self.value_ = value


class Object(Value):
  
  def __init__(self):
    super(Object, self).__init__()


class String(Value):
  
  def __init__(self, value):
    super(String, self).__init__()
    self.value_ = value
  
  def __str__(self):
    return '"%s"' % self.value_


class Dictionary(Object):
  
  def __init__(self):
    super(Dictionary, self).__init__()
    self.map_ = { }
  
  def map(self):
    return self.map_

  def __setitem__(self, key, value):
    self.map()[key] = value

  def __str__(self):
    items = ", ".join([ '%s: %s' % (k, v) for (k, v) in self.map().items() ])
    return "{%s}" % items
