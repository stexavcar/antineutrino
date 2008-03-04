
class SyntaxTree(object):
  pass


class Documentation(SyntaxTree):
  
  def __init__(self, value):
    super(Documentation, self).__init__()
    self.value_ = value


class Symbol(SyntaxTree):

  def __init__(self, name):
    super(Symbol, self).__init__()
    self.name_ = name
  
  def name(self):
    return self.name_
  

class Arguments(SyntaxTree):
  
  def __init__(self, args, keywords):
    super(Arguments, self).__init__()
    self.args_ = args
    self.keywords_ = keywords


class File(SyntaxTree):
  
  def __init__(self, defs):
    super(File, self).__init__()
    self.defs_ = defs
  
  def accept(self, visitor):
    visitor.visit_file(self)
  
  def traverse(self, visitor):
    for d in self.defs_:
      d.accept(visitor)


class Definition(SyntaxTree):
  
  def __init__(self, name, value):
    self.name_ = name
    self.value_ = value

  def name(self):
    return self.name_
  
  def value(self):
    return self.value_

  def accept(self, visitor):
    visitor.visit_definition(self)
  
  def traverse(self, visitor):
    self.value_.accept(visitor)


# --- D e c l a r a t i o n s ---


class Declaration(SyntaxTree):
  
  def __init__(self, doc, name):
    super(Declaration, self).__init__()
    self.doc_ = doc
    self.name_ = name


class MethodDeclaration(Declaration):
  
  def __init__(self, doc, modifiers, name, args, body):
    super(MethodDeclaration, self).__init__(doc, name)
    self.modifiers_ = modifiers
    self.args_ = args
    self.body_ = body


# --- E x p r e s s i o n s ---


class Expression(SyntaxTree):
  
  def __init__(self):
    super(Expression, self).__init__()
  
  def is_sequence(self):
    return False


class Lambda(Expression):
  
  def __init__(self, params, body):
    self.params_ = params
    self.body_ = body


class Sequence(Expression):
  
  def __init__(self, exprs):
    super(Sequence, self).__init__()
    assert len(exprs) > 1
    self.exprs_ = exprs

  def is_sequence(self):
    return False
  
  def make(exprs):
    if len(exprs) == 0: return Void()
    elif len(exprs) == 1: return exprs[0]
    else: return Sequence(exprs)
  make = staticmethod(make)


class Protocol(Expression):
  
  def __init__(self, modifiers, name, s, members):
    super(Protocol, self).__init__()
    self.modifiers_ = modifiers
    self.name_ = name
    self.super_ = s
    self.members_ = members


class Call(Expression):
  
  def __init__(self, recv, fun, args):
    super(Call, self).__init__()
    self.recv_ = recv
    self.fun_ = fun
    self.args_ = args


class Invoke(Expression):
  
  def __init__(self, recv, name, args):
    super(Invoke, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args


class Instantiate(Expression):
  
  def __init__(self, recv, name, args, terms):
    super(Instantiate, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args
    self.terms_ = terms


class Return(Expression):
  
  def __init__(self, value):
    super(Return, self).__init__()
    self.value_ = value
  
  def write_to(self, out):
    [sub] = out.format("return @<")
    self.value_.write_to(sub)


class Raise(Expression):
  
  def __init__(self, name, args):
    super(Raise, self).__init__()
    self.name_ = name
    self.args_ = args


class This(Expression):
  
  def __init__(self):
    super(This, self).__init__()


class Null(Expression):
  
  def __init__(self):
    super(Null, self).__init__()


class Void(Expression):
  
  def __init__(self):
    super(Void, self).__init__()


class Conditional(Expression):
  
  def __init__(self, cond, then_part, else_part):
    self.cond_ = cond
    self.then_part_ = then_part
    self.else_part_ = else_part


class LocalDefinition(Expression):
  
  def __init__(self, name, value, body):
    self.name_ = name
    self.value_ = value
    self.body_ = body


class Literal(Expression):
  
  def __init__(self, value):
    super(Literal, self).__init__()
    self.value_ = value


class Identifier(Expression):
  
  def __init__(self, name):
    super(Identifier, self).__init__()
    self.name_ = name


class Global(Identifier):
  
  def __init__(self, name):
    super(Global, self).__init__(name)


class Local(Identifier):
  
  def __init__(self, symbol):
    super(Local, self).__init__(symbol.name())
    self.symbol_ = symbol

# --- V i s i t o r ---

class Visitor(object):
  
  def visit_tree(self, that):
    that.traverse(self)
  
  def visit_file(self, that):
    self.visit_tree(that)

  def visit_definition(self, that):
    self.visit_tree(that)
