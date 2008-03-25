import array

class Address(object):

  def __init__(self, heap, value):
    self.heap_ = heap
    self.value_ = value

  def tag_as_object(self, anchor):
    return (self.value_ << 2) | 0x1

  def heap(self):
    return self.heap_

  def __setitem__(self, addr, value):
    index = Heap.kHeaderSize + self.value_ + addr
    self.heap().set_item(self.value_, index, value)


class Raw(object):

  def __init__(self, value):
    self.value_ = value

  def get_int_value(self, anchor, heap):
    return self.value_


class Heap(object):

  kMagicNumberOffset = 0
  kVersionOffset     = 1
  kHeapSizeOffset    = 2
  kRootCountOffset   = 3
  kRootsOffset       = 4
  kHeaderSize        = 5

  kCurrentVersion    = 2
  kRootCount         = 55
  
  def __init__(self, roots):
    self.data_ = array.array('l')
    self.cursor_ = 0
    # Hex literals in python don't automatically overflow into
    # negative numbers (which is a good thing, really) so we manually
    # overflow it so it will fit in the array
    magic = 0xFABACEAE - pow(2, 32)
    self[Heap.kMagicNumberOffset] = Raw(magic)
    self[Heap.kVersionOffset] = Raw(Heap.kCurrentVersion)
    self[Heap.kRootCountOffset] = Raw(Heap.kRootCount)
    self[Heap.kRootsOffset] = roots

  def allocate(self, size, layout):
    addr = self.cursor_
    self.cursor_ += size
    self[addr + Heap.kHeaderSize] = layout
    return Address(self, addr)

  def data(self):
    return self.data_

  def __setitem__(self, key, value):
    if len(self.data()) <= key: self.extend_to(key + 1)
    self.data()[key] = value.get_int_value(0, self)
  
  def set_item(self, anchor, key, value):
    if len(self.data()) <= key: self.extend_to(key + 1)
    self.data()[key] = value.get_int_value(anchor, self)

  def extend_to(self, size):
    while len(self.data()) < size:
      self.data().append(0)

  def store(self, name):
    self[Heap.kHeapSizeOffset] = Raw(self.cursor_)
    f = open(name, "w")
    self.data_.write(f)
    f.close()
