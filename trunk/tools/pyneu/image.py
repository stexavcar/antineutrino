import array

class Heap(object):
  
  kInitialSize = 1024
  kCurrentVersion = 2
  kRootCount = 51
  
  def __init__(self):
    self.data_ = array.array('L')
    self.extend_to(Heap.kInitialSize)
    self[0] = 4206546606L
    self[1] = Heap.kCurrentVersion
    self[3] = Heap.kRootCount
  
  def data(self):
    return self.data_
  
  def __setitem__(self, key, value):
    self.data()[key] = value
  
  def extend_to(self, size):
    while len(self.data()) < size:
      self.data().append(0)

  def store(self, name):
    f = open(name, "w")
    self.data_.write(f)
    f.close()
