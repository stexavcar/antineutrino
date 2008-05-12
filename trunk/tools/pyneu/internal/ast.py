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


class Symbol(object):

  def __init__(self, name):
    super(Symbol, self).__init__()
    self.name_ = name
    self.image_ = None

  def name(self):
    return self.name_

  def __repr__(self):
    return "$%s" % self.name_

  def quote(self):
    # We need to preserve the identity of this symbol when quoting,
    # so the result has to be cached so we can return if this symbol
    # is quoted again later
    if not self.image_:
      self.image_ = values.Symbol(self.name_)
    return self.image_


class Arguments(SyntaxTree):

  def __init__(self, args, keywords, is_accessor):
    super(Arguments, self).__init__()
    for arg in args: assert isinstance(arg, SyntaxTree)
    self.args_ = args
    self.keywords_ = keywords
    self.is_accessor_ = is_accessor
  
  def extend(self, value):
    return Arguments(self.args_ + [value], self.keywords_, self.is_accessor_)

  def clone_with_args(self, args):
    return Arguments(args, self.keywords_, self.is_accessor_)

  def __len__(self):
    return len(self.args_)
  
  def is_accessor(self):
    return self.is_accessor_

  def accept(self, visitor):
    visitor.visit_arguments(self)
  
  def keywords(self):
    return self.keywords_.keys()
  
  def arguments(self):
    return self.args_

  def traverse(self, visitor):
    for arg in self.args_:
      arg.accept(visitor)
  
  def construct(self):
    args = make_tuple([ s.construct() for s in self.args_ ])
    return make_construct("Arguments", [args, values.LiteralExpression(values.EMPTY_TUPLE)])

  def quote(self):
    args = [ s.quote() for s in self.args_ ]
    return values.Arguments(args, self.keywords_)


class Parameters(SyntaxTree):

  def __init__(self, posc, params, is_accessor):
    super(Parameters, self).__init__()
    self.posc_ = posc
    self.params_ = params
    self.is_accessor_ = is_accessor

  def params(self):
    return self.params_
  
  def extend(self, that):
    return Parameters(
        self.posc_ + that.posc_,
        self.params_ + that.params_,
        self.is_accessor_ and that.is_accessor_
    )
  
  def keywords(self):
    return self.params_[self.posc_:]
  
  def is_accessor(self):
    return self.is_accessor_

  def __len__(self):
    return len(self.params_)
  
  def quote(self):
    params = [ p.quote() for p in self.params_ ]
    return values.Parameters(self.posc_, params)


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

  def __init__(self, modifiers, name, value):
    assert type(name) is unicode
    self.modifiers_ = modifiers
    self.name_ = name
    self.value_ = value

  def modifiers(self):
    return self.modifiers_

  def name(self):
    return self.name_

  def value(self):
    return self.value_

  def accept(self, visitor):
    visitor.visit_definition(self)

  def traverse(self, visitor):
    if self.value_:
      self.value_.accept(visitor)


# --- D e c l a r a t i o n s ---


class Declaration(SyntaxTree):

  def __init__(self, doc, name):
    super(Declaration, self).__init__()
    self.doc_ = doc
    self.name_ = name


class Method(Declaration):

  def __init__(self, doc, modifiers, klass, name, params, body):
    super(Method, self).__init__(doc, name)
    assert not name is None
    self.modifiers_ = modifiers
    full_name = "%s.%s" % (klass, name)
    self.lambda_ = Lambda(modifiers, full_name, params, body, False)

  def is_static(self):
    return parser.STATIC in self.modifiers_

  def quote(self):
    params = self.lambda_.params()
    argc = len(params)
    is_accessor = params.is_accessor()
    keywords = [ k.name() for k in params.keywords() ]
    selector = values.Selector(self.name_, argc, keywords, is_accessor)
    lam = self.lambda_.quote()
    return values.MethodExpression(selector, lam, values.FALSE)

  def evaluate(self):
    params = self.lambda_.params()
    argc = len(params)
    is_accessor = params.is_accessor()
    keywords = [ k.name() for k in params.keywords() ]
    selector = values.Selector(self.name_, argc, keywords, is_accessor)
    lam = self.lambda_.evaluate()
    return values.Method(selector, lam, self.is_static())


# --- E x p r e s s i o n s ---


class Expression(SyntaxTree):

  def __init__(self):
    super(Expression, self).__init__()

  def is_sequence(self):
    return False

  def is_identifier(self):
    return False
  
  def is_invoke(self):
    return False


class Lambda(Expression):

  def __init__(self, modifiers, name, params, body, is_local):
    super(Lambda, self).__init__()
    self.modifiers_ = modifiers
    self.name_ = name
    self.params_ = params
    self.is_local = is_local
    if body is None:
      if parser.INTERNAL in self.modifiers_:
        index = consts.builtin_functions[name].index
        self.body_ = Return(InternalCall(len(params), index))
    else:
      self.body_ = body

  def accept(self, visitor):
    visitor.visit_lambda(self)

  def traverse(self, visitor):
    self.body().accept(visitor)

  def params(self):
    return self.params_

  def body(self):
    return self.body_

  def quote(self):
    params = self.params().quote()
    body = self.body().quote()
    return values.LambdaExpression(params, body, self.is_local)

  def evaluate(self):
    body = self.quote()
    return values.Lambda(len(self.params_), body)


class Task(Expression):

  def __init__(self, lam):
    super(Task, self).__init__()
    self.lambda_ = lam

  def accept(self, visitor):
    visitor.visit_task(self)

  def traverse(self, visitor):
    self.body_.accept(visitor)

  def quote(self):
    return values.TaskExpression(self.lambda_.quote())


class Quote(Expression):

  def __init__(self, value, unquotes):
    super(Quote, self).__init__()
    self.value_ = value
    self.unquotes_ = unquotes

  def quote(self):
    return self.value_.construct()


class Unquote(Expression):

  def __init__(self, index):
    super(Unquote, self).__init__()
    self.index_ = index

  def quote(self):
    return values.UnquoteExpression(self.index_)


class Sequence(Expression):

  def __init__(self, exprs):
    super(Sequence, self).__init__()
    assert len(exprs) > 1
    self.exprs_ = exprs

  def quote(self):
    return values.SequenceExpression([ e.quote() for e in self.exprs_ ])

  def accept(self, visitor):
    visitor.visit_sequence(self)

  def traverse(self, visitor):
    for expr in self.exprs_:
      expr.accept(visitor)

  def is_sequence(self):
    return False

  def make(exprs):
    if len(exprs) == 0: return Literal(Void())
    elif len(exprs) == 1: return exprs[0]
    else: return Sequence(exprs)
  make = staticmethod(make)


class Protocol(Expression):

  def __init__(self, modifiers, name, super_name, members):
    super(Protocol, self).__init__()
    self.modifiers_ = modifiers
    self.name_ = name
    self.super_name_ = super_name
    self.members_ = members
    self.image_ = None

  def accept(self, visitor):
    visitor.visit_protocol(self)

  def traverse(self, visitor):
    for member in self.members_:
      member.accept(visitor)

  def super_name(self):
    return self.super_name_

  def name(self):
    return self.name_

  def modifiers(self):
    return self.modifiers_

  def evaluate(self):
    assert not self.image_
    members = [ m.evaluate() for m in self.members_ if not m.is_static() ]
    static_members = [ m.evaluate() for m in self.members_ if m.is_static() ]
    result = values.Protocol(self.name_, members, static_members)
    self.image_ = result
    return result

  def quote(self):
    members = [ member.quote() for member in self.members_ ]
    if self.super_name_: sup = Global(self.super_name_).quote()
    else: sup = values.VOID
    return values.ProtocolExpression(self.name_, members, sup)

class Call(Expression):

  def __init__(self, recv, fun, args):
    super(Call, self).__init__()
    self.recv_ = recv
    self.fun_ = fun
    self.args_ = args

  def accept(self, visitor):
    visitor.visit_call(self)

  def traverse(self, visitor):
    self.recv_.accept(visitor)
    self.fun_.accept(visitor)
    self.args_.accept(visitor)

  def quote(self):
    recv = self.recv_.quote()
    fun = self.fun_.quote()
    args = self.args_.quote()
    return values.CallExpression(recv, fun, args)


def make_construct(name, arg_list):
  sel = values.Selector("new", len(arg_list), [], False)
  args = values.Arguments(arg_list, {})
  return values.InvokeExpression(values.GlobalVariable(name), sel, args)


def make_tuple(args):
  return values.TupleExpression(args)


class Invoke(Expression):

  def __init__(self, recv, name, args):
    super(Invoke, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args
  
  def is_invoke(self):
    return True

  def accept(self, visitor):
    visitor.visit_invoke(self)
  
  def recv(self):
    return self.recv_
  
  def name(self):
    return self.name_
  
  def args(self):
    return self.args_

  def traverse(self, visitor):
    self.recv_.accept(visitor)
    self.args_.accept(visitor)
  
  def construct(self):
    recv = self.recv_.construct()
    args = self.args_.construct()
    is_accessor = self.args_.is_accessor()
    sel = values.LiteralExpression(values.Selector(self.name_, len(self.args_), self.args_.keywords(), is_accessor))
    return make_construct("InvokeExpression", [recv, sel, args])

  def quote(self):
    recv = self.recv_.quote()
    args = self.args_.quote()
    is_accessor = self.args_.is_accessor()
    sel = values.Selector(self.name_, len(self.args_), self.args_.keywords(), is_accessor)
    return values.InvokeExpression(recv, sel, args)


class Instantiate(Expression):

  def __init__(self, recv, name, args, terms):
    super(Instantiate, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args
    self.terms_ = terms

  def accept(self, visitor):
    visitor.visit_instantiate(self)

  def traverse(self, visitor):
    self.recv_.accept(visitor)
    self.args_.accept(visitor)
    for (k, v) in self.terms_:
      v.accept(visitor)

  def quote(self):
    recv = self.recv_.quote()
    args = self.args_.quote()
    terms = { }
    for (k, v) in self.terms_:
      terms[k] = v.quote()
    return values.InstantiateExpression(recv, self.name_, args, terms)


class Tuple(Expression):

  def __init__(self, exprs):
    super(Tuple, self).__init__()
    self.exprs_ = exprs
  
  def accept(self, visitor):
    pass
  
  def evaluate(self):
    vals = [ e.evaluate() for e in self.exprs_ ]
    return values.Tuple(entries = vals)

  def quote(self):
    expr = [ e.quote() for e in self.exprs_ ]
    return values.TupleExpression(expr)


class Return(Expression):

  def __init__(self, value):
    super(Return, self).__init__()
    self.value_ = value

  def accept(self, visitor):
    visitor.visit_return(self)

  def traverse(self, visitor):
    self.value_.accept(visitor)

  def quote(self):
    return values.ReturnExpression(self.value_.quote())


class Yield(Expression):

  def __init__(self, value):
    super(Yield, self).__init__()
    self.value_ = value

  def accept(self, visitor):
    visitor.visit_yield(self)

  def traverse(self, visitor):
    self.value_.accept(visitor)

  def quote(self):
    return values.YieldExpression(self.value_.quote())


class DoOnExpression(Expression):

  def __init__(self, value, clauses):
    super(DoOnExpression, self).__init__()
    self.value_ = value
    self.clauses_ = clauses

  def quote(self):
    clauses = [ c.quote() for c in self.clauses_ ]
    value = self.value_.quote()
    return values.DoOnExpression(value, clauses)


class OnClause(SyntaxTree):

  def __init__(self, name, lam):
    super(OnClause, self).__init__()
    self.name_ = name
    self.lambda_ = lam

  def quote(self):
    return values.OnClause(self.name_, self.lambda_.quote())


class Raise(Expression):

  def __init__(self, name, args):
    super(Raise, self).__init__()
    self.name_ = name
    self.args_ = args

  def accept(self, visitor):
    visitor.visit_raise(self)

  def traverse(self, visitor):
    self.args_.accept(visitor)

  def quote(self):
    args = self.args_.quote()
    return values.RaiseExpression(self.name_, args)


class This(Expression):

  def __init__(self):
    super(This, self).__init__()

  def accept(self, visitor):
    visitor.visit_this(self)

  def traverse(self, visitor):
    pass

  def quote(self):
    return values.ThisExpression()


class Super(Expression):
  
  def __init__(self, value):
    super(Super, self).__init__()
    self.value_ = value
  
  def accept(self, visitor):
    visitor.visit_super(self)
  
  def traverse(self, visitor):
    self.value_.accept(visitor)
  
  def quote(self):
    value = self.value_.quote()
    return values.SuperExpression(value)


class Null(Expression):

  def __init__(self):
    super(Null, self).__init__()

  def evaluate(self):
    return self.quote()

  def quote(self):
    return values.NULL


class Void(Expression):

  def __init__(self):
    super(Void, self).__init__()

  def evaluate(self):
    return self.quote()

  def quote(self):
    return values.VOID


class Thrue(Expression):

  def __init__(self):
    super(Thrue, self).__init__()

  def evaluate(self):
    return self.quote()

  def quote(self):
    return values.TRUE


class Fahlse(Expression):

  def __init__(self):
    super(Fahlse, self).__init__()

  def evaluate(self):
    return self.quote()

  def quote(self):
    return values.FALSE


class Conditional(Expression):

  def __init__(self, cond, then_part, else_part):
    self.cond_ = cond
    self.then_part_ = then_part
    self.else_part_ = else_part

  def accept(self, visitor):
    visitor.visit_conditional(self)

  def traverse(self, visitor):
    self.cond_.accept(visitor)
    self.then_part_.accept(visitor)
    self.else_part_.accept(visitor)

  def quote(self):
    cond = self.cond_.quote()
    then_part = self.then_part_.quote()
    else_part = self.else_part_.quote()
    return values.ConditionalExpression(cond, then_part, else_part)


class While(Expression):

  def __init__(self, cond, body):
    super(Expression, self).__init__()
    self.cond_ = cond
    self.body_ = body
  
  def accept(self, visitor):
    visitor.visit_while(self)
  
  def traverse(self, visitor):
    self.cond_.accept(visitor)
    self.body_.accept(visitor)
  
  def quote(self):
    cond = self.cond_.quote()
    body = self.body_.quote()
    return values.WhileExpression(cond, body)


class InternalCall(Expression):

  def __init__(self, argc, index):
    super(InternalCall, self).__init__()
    self.argc_ = argc
    self.index_ = index

  def accept(self, visitor):
    visitor.visit_internal_call(self)

  def traverse(self, visitor):
    pass

  def quote(self):
    return values.InternalCall(self.argc_, self.index_)


class LocalDefinition(Expression):

  def __init__(self, symbol, value, body, type):
    super(LocalDefinition, self).__init__()
    self.symbol_ = symbol
    self.value_ = value
    self.body_ = body
    self.type_ = type

  def accept(self, visitor):
    visitor.visit_local_definition(self)

  def traverse(self, visitor):
    self.value_.accept(visitor)
    self.body_.accept(visitor)

  def quote(self):
    symbol = self.symbol_.quote()
    value = self.value_.quote()
    body = self.body_.quote()
    return values.LocalDefinition(symbol, value, body, self.type_)


class Assignment(Expression):

  def __init__(self, symbol, value):
    super(Assignment, self).__init__()
    self.symbol_ = symbol
    self.value_ = value
  
  def quote(self):
    symbol = self.symbol_.quote()
    value = self.value_.quote()
    return values.Assignment(symbol, value)


def to_literal(value):
  if type(value) in [unicode, str]: return values.String(value)
  elif type(value) in [int, long]: return values.Smi(value)
  if isinstance(value, SyntaxTree): return value.quote()
  else: return value


class Literal(Expression):

  def __init__(self, value):
    super(Literal, self).__init__()
    self.value_ = value

  def accept(self, visitor):
    visitor.visit_literal(self)

  def traverse(self, visitor):
    pass
  
  def evaluate(self):
    return to_literal(self.value_)

  def construct(self):
    return make_construct("LiteralExpression", [ self.quote() ])

  def quote(self):
    return values.LiteralExpression(to_literal(self.value_))


class Interpolate(Expression):

  def __init__(self, terms):
    super(Interpolate, self).__init__()
    self.terms_ = terms

  def accept(self, visitor):
    visitor.visit_interpolate(self)

  def traverse(self, visitor):
    for term in self.terms_:
      if isinstance(term, Expression):
        term.accept(visitor)

  def quote(self):
    terms = [ to_literal(t) for t in self.terms_ ]
    return values.InterpolateExpression(terms)

class Identifier(Expression):

  def __init__(self, name):
    super(Identifier, self).__init__()
    self.name_ = name

  def name(self):
    return self.name_

  def is_identifier(self):
    return True


class Global(Identifier):

  def __init__(self, name):
    super(Global, self).__init__(name)

  def accept(self, visitor):
    visitor.visit_global(self)

  def traverse(self, visitor):
    pass

  def quote(self):
    return values.GlobalVariable(self.name())


class Local(Identifier):

  def __init__(self, symbol):
    super(Local, self).__init__(symbol.name())
    self.symbol_ = symbol
  
  def symbol(self):
    return self.symbol_

  def accept(self, visitor):
    visitor.visit_local(self)

  def traverse(self, visitor):
    pass

  def quote(self):
    return values.LocalVariable(self.symbol_.quote())

# --- V i s i t o r ---

class Visitor(object):

  def visit_tree(self, that):
    that.traverse(self)

  def visit_file(self, that):
    self.visit_tree(that)

  def visit_definition(self, that):
    self.visit_tree(that)

  def visit_lambda(self, that):
    self.visit_tree(that)

  def visit_protocol(self, that):
    self.visit_tree(that)

  def visit_return(self, that):
    self.visit_tree(that)

  def visit_internal_call(self, that):
    self.visit_tree(that)

  def visit_call(self, that):
    self.visit_tree(that)

  def visit_this(self, that):
    self.visit_tree(that)

  def visit_global(self, that):
    self.visit_tree(that)

  def visit_arguments(self, that):
    self.visit_tree(that)

  def visit_invoke(self, that):
    self.visit_tree(that)

  def visit_local(self, that):
    self.visit_tree(that)

  def visit_sequence(self, that):
    self.visit_tree(that)

  def visit_raise(self, that):
    self.visit_tree(that)

  def visit_literal(self, that):
    self.visit_tree(that)

  def visit_conditional(self, that):
    self.visit_tree(that)

  def visit_interpolate(self, that):
    self.visit_tree(that)

  def visit_local_definition(self, that):
    self.visit_tree(that)

  def visit_instantiate(self, that):
    self.visit_tree(that)
