import consts
import image
import parser
import values

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
  
  def quote(self):
    return values.Symbol(self.name_)
  

class Arguments(SyntaxTree):
  
  def __init__(self, args, keywords):
    super(Arguments, self).__init__()
    self.args_ = args
    self.keywords_ = keywords
  
  def quote(self):
    return values.Arguments()


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
    assert type(name) is unicode
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


class Method(Declaration):
  
  def __init__(self, doc, modifiers, name, params, body):
    super(Method, self).__init__(doc, name)
    assert not name is None
    self.modifiers_ = modifiers
    self.name_ = name
    self.lambda_ = Lambda(modifiers, name, params, body)
  
  def evaluate(self):
    argc = values.Smi(len(self.lambda_.params()))
    selector = values.Selector(values.String(self.name_), argc)
    lam = self.lambda_.evaluate()
    return values.Method(selector, lam)


# --- E x p r e s s i o n s ---


class Expression(SyntaxTree):
  
  def __init__(self):
    super(Expression, self).__init__()
  
  def is_sequence(self):
    return False


class Lambda(Expression):
  
  def __init__(self, modifiers, name, params, body):
    self.modifiers_ = modifiers
    self.name_ = name
    self.params_ = params
    if body is None:
      if parser.INTERNAL in self.modifiers_:
        index = consts.builtin_functions[name]
        self.body_ = Return(InternalCall(len(params), index))
      else:
        assert parser.NATIVE in self.modifiers_
        self.body_ = Return(NativeCall(len(params), name))
    else:
      self.body_ = body
  
  def params(self):
    return self.params_
  
  def body(self):
    return self.body_
  
  def evaluate(self):
    argc = len(self.params())
    body = self.body().quote()
    return values.Lambda(argc, body)


class Sequence(Expression):
  
  def __init__(self, exprs):
    super(Sequence, self).__init__()
    assert len(exprs) > 1
    self.exprs_ = exprs
  
  def quote(self):
    return values.SequenceExpression([ e.quote() for e in self.exprs_ ])

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
  
  def evaluate(self):
    members = [ member.evaluate() for member in self.members_ ]
    return values.Protocol(values.String(self.name_), members)


class Call(Expression):
  
  def __init__(self, recv, fun, args):
    super(Call, self).__init__()
    self.recv_ = recv
    self.fun_ = fun
    self.args_ = args
  
  def quote(self):
    recv = self.recv_.quote()
    fun = self.fun_.quote()
    args = self.args_.quote()
    return values.CallExpression(recv, fun, args)


class Invoke(Expression):
  
  def __init__(self, recv, name, args):
    super(Invoke, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args
  
  def quote(self):
    recv = self.recv_.quote()
    args = self.args_.quote()
    return values.InvokeExpression(recv, self.name_, args)


class Instantiate(Expression):
  
  def __init__(self, recv, name, args, terms):
    super(Instantiate, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args
    self.terms_ = terms
  
  def quote(self):
    recv = self.recv_.quote()
    args = self.args_.quote()
    terms = [ (k, v.quote()) for (k, v) in self.terms_ ]
    return values.InstantiateExpression(recv, self.name_, args, terms)


class Return(Expression):
  
  def __init__(self, value):
    super(Return, self).__init__()
    assert not value is None
    self.value_ = value
  
  def quote(self):
    return values.ReturnExpression(self.value_.quote())


class Raise(Expression):
  
  def __init__(self, name, args):
    super(Raise, self).__init__()
    self.name_ = name
    self.args_ = args


class This(Expression):
  
  def __init__(self):
    super(This, self).__init__()
  
  def quote(self):
    return values.ThisExpression()


class Null(Expression):
  
  def __init__(self):
    super(Null, self).__init__()
  
  def quote(self):
    return values.NULL


class Void(Expression):
  
  def __init__(self):
    super(Void, self).__init__()
  
  def quote(self):
    return values.VOID


class Conditional(Expression):
  
  def __init__(self, cond, then_part, else_part):
    self.cond_ = cond
    self.then_part_ = then_part
    self.else_part_ = else_part
  
  def quote(self):
    cond = self.cond_.quote()
    then_part = self.then_part_.quote()
    else_part = self.else_part_.quote()
    return values.ConditionalExpression(cond, then_part, else_part)


class InternalCall(Expression):
  
  def __init__(self, argc, index):
    super(InternalCall, self).__init__()
    self.argc_ = argc
    self.index_ = index
  
  def quote(self):
    return values.InternalCall(self.argc_, self.index_)


class NativeCall(Expression):
  
  def __init__(self, argc, name):
    super(NativeCall, self).__init__()
    self.argc_ = argc
    self.name_ = name
  
  def quote(self):
    return values.NativeCall(self.argc_, self.name_)


class LocalDefinition(Expression):
  
  def __init__(self, name, value, body):
    self.name_ = name
    self.value_ = value
    self.body_ = body
  
  def quote(self):
    value = self.value_.quote()
    body = self.body_.quote()
    return values.LocalDefinition(self.name_, value, body)


class Literal(Expression):
  
  def __init__(self, value):
    super(Literal, self).__init__()
    self.value_ = value
  
  def quote(self):
    return values.LiteralExpression(self.value_)


class Identifier(Expression):
  
  def __init__(self, name):
    super(Identifier, self).__init__()
    self.name_ = name
  
  def name(self):
    return self.name_


class Global(Identifier):
  
  def __init__(self, name):
    super(Global, self).__init__(name)
  
  def quote(self):
    return values.GlobalExpression(self.name())


class Local(Identifier):
  
  def __init__(self, symbol):
    super(Local, self).__init__(symbol.name())
    self.symbol_ = symbol
  
  def quote(self):
    return values.LocalExpression(self.symbol_.quote())

# --- V i s i t o r ---

class Visitor(object):
  
  def visit_tree(self, that):
    that.traverse(self)
  
  def visit_file(self, that):
    self.visit_tree(that)

  def visit_definition(self, that):
    self.visit_tree(that)
