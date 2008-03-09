import ast
import consts
import image
import parser
import reader
import values


def compile(files, out):
  asts = load_files(files)
  toplevel = values.Dictionary()
  load_into(asts, toplevel)
  roots = values.Tuple(length = image.Heap.kRootCount)
  roots[consts.roots.toplevel.index] = toplevel
  heap = image.Heap(roots)
  heap.store(out)


def load_into(asts, toplevel):
  load_visitor = LoadVisitor(toplevel)
  for ast in asts:
    ast.accept(load_visitor)


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
  
  def __init__(self, toplevel):
    self.toplevel_ = toplevel
  
  def toplevel(self):
    return self.toplevel_

  def visit_tree(self, that):
    assert False

  def visit_file(self, that):
    that.traverse(self)
  
  def visit_definition(self, that):
    name = values.String(that.name())
    value = that.value().evaluate()
    self.toplevel()[name] = value
