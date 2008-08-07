import ast
import consts
import image
import parser
import reader
import values


def compile(files, out):
  asts = load_files(files)
  toplevel = values.HashMap()
  roots = values.Tuple(length = image.Heap.kRootCount)
  load_into(asts, toplevel, roots)
  heap = image.Heap(roots)
  heap.store(out)


def load_into(asts, toplevel, roots):
  roots[consts.roots.toplevel.index] = toplevel
  load_visitor = LoadVisitor(toplevel)
  for ast in asts:
    ast.accept(load_visitor)
  resolve_visitor = ResolveVisitor(toplevel, roots)
  for ast in asts:
    ast.accept(resolve_visitor)


def load_files(files):
  """Loads the specified list of files into the heap"""
  trees = [ ]
  for f in files:
    doc = reader.TextDocument.read(f)
    parse = parser.Parser.open(doc)
    tree = parse.program()
    trees.append(tree)
  return trees


class LoadVisitor(ast.Visitor):
  """Installs the toplevel declarations as values in the global namespace"""

  def __init__(self, toplevel):
    self.toplevel_ = toplevel

  def toplevel(self):
    return self.toplevel_

  def visit_tree(self, that):
    assert False

  def visit_file(self, that):
    that.traverse(self)

  def visit_definition(self, that):
    if parser.CHANNEL in that.modifiers():
      self.toplevel()[that.name()] = values.Channel(that.name())
    else:
      value = that.value().evaluate()
      self.toplevel()[that.name()] = value


class ResolveVisitor(ast.Visitor):
  """Resolves "hard" references and installs entries in the roots tuple"""

  def __init__(self, toplevel, roots):
    self.toplevel_ = toplevel
    self.roots_ = roots

  def toplevel(self):
    return self.toplevel_

  def roots(self):
    return self.roots_

  def visit_lambda(self, that):
    pass

  def visit_protocol(self, that):
    # Find the super protocol value
    image = that.image_
    super_name = that.super_name()
    if super_name:
      image.super_ = self.toplevel()[super_name]
    else:
      if that.name() == "Object":
        image.super_ = values.NULL
      else:
        image.super_ = self.toplevel()["Object"]

    # If this protocol is internal, install it in the root set
    if parser.INTERNAL in that.modifiers():
      layout_name = "%s_layout" % consts.upper_to_lower[that.name()]
      index = consts.roots[layout_name].index
      self.roots()[index] = image

    # For all methods in this protocol set the appropriate signature
    for member in image.methods():
      member.set_signature(image)

    static_protocol = image.static_protocol()
    for method in static_protocol.methods():
      method.set_signature(static_protocol)
    if that.name() == "Object":
      static_protocol.set_super(self.toplevel()["Protocol"])
    else:
      static_protocol.set_super(image.super_.static_protocol())
