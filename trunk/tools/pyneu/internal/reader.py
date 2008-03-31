"""Input management for the py-neutrino compiler"""

import codecs

class TextDocument(object):
  """An encapsulated text file which keeps track of the current
  position"""

  def __init__(self, name, contents):
    self.name = name
    self.contents = contents

  def __str__(self):
    return "#<a TextDocument: %s>" % self.name
  
  def __getitem__(self, index):
    if index >= len(self.contents): return ""
    return self.contents[index]
  
  def __len__(self):
    return len(self.contents)
  
  def get_line_of_offset(self, offset):
    line = 0
    for i in xrange(offset):
      if self[i] == "\n":
        line += 1
    return line
  
  def get_line(self, line):
    return self.contents.split("\n")[line]
  
  def get_line_start(self, line):
    current = 0
    for i in xrange(len(self)):
      if line == current: return i
      if self[i] == "\n":
        current += 1
    return len(self)
  
  def read(name):
    contents = codecs.open(name, "r", "utf-8").read()
    return TextDocument(name, contents)
  read = staticmethod(read)


class Reader(object):
  
  def __init__(self, doc):
    self.doc = doc
    self.cursor = 0
  
  def __str__(self):
    return "#<a Reader: %s>" % self.doc.name
  
  def has_more(self):
    return self.cursor < len(self.doc)
  
  def has_next(self):
    return self.cursor + 1 < len(self.doc)
  
  def position(self):
    return self.cursor
  
  def set_position(self, value):
    self.cursor = value
  
  def current(self):
    return self.doc[self.cursor]
  
  def next(self):
    return self.doc[self.cursor + 1]
  
  def advance(self):
    self.cursor += 1
