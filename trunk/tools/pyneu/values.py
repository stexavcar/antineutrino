from consts import values, fields, roots
import image

class Value(object):
  pass


class Smi(Value):
  
  def __init__(self, value):
    super(Smi, self).__init__()
    self.value_ = value
  
  def get_int_value(self, heap):
    return (self.value_ << 2)


class Object(Value):
  
  def __init__(self):
    super(Object, self).__init__()
    self.addresses = { }
  
  def get_int_value(self, heap):
    if not heap in self.addresses:
      obj = self.allocate(heap)
      assert isinstance(obj, image.Address)
      self.addresses[heap] = obj
    return self.addresses[heap].tag_as_object()


class String(Object):
  
  def __init__(self, value):
    super(String, self).__init__()
    assert not value is None
    self.value_ = value
  
  def __str__(self):
    return '"%s"' % self.value_
  
  def allocate(self, heap):
    length = len(self.value_)
    size = fields.ImageString_HeaderSize + length
    result = heap.allocate(size, Smi(values.String.index))
    result[fields.ImageString_LengthOffset] = image.Raw(length)
    for i in xrange(length):
      char = self.value_[i]
      result[fields.ImageString_HeaderSize + i] = image.Raw(ord(char))
    return result


class Tuple(Object):
  
  def __init__(self, length = None, entries = None):
    super(Tuple, self).__init__()
    if not length is None:
      self.entries_ = [ Smi(0) ] * length
    elif not entries is None:
      self.entries_ = entries
    else:
      assert False
  
  def __setitem__(self, index, item):
    assert isinstance(item, Value)
    self.entries_[index] = item
  
  def allocate(self, heap):
    size = fields.ImageTuple_HeaderSize + len(self.entries_)
    result = heap.allocate(size, Smi(values.Tuple.index))
    result[fields.ImageTuple_LengthOffset] = image.Raw(len(self.entries_))
    for i in xrange(len(self.entries_)):
      result[fields.ImageTuple_HeaderSize + i] = self.entries_[i]
    return result


EMPTY_TUPLE = Tuple(length = 0)


class Signature(Object):
  
  def __init__(self):
    super(Signature, self).__init__()
  
  def allocate(self, heap):
    result = heap.allocate(fields.ImageSignature_Size, Smi(values.Signature.index))
    result[fields.ImageSignature_ParametersOffset] = EMPTY_TUPLE
    return result


EMPTY_SIGNATURE = Signature()


class Selector(Object):
  
  def __init__(self, name, argc):
    super(Selector, self).__init__()
    self.name_ = name
    self.argc_ = argc
  
  def allocate(self, heap):
    result = heap.allocate(fields.ImageSelector_Size, Smi(values.Selector.index))
    result[fields.ImageSelector_NameOffset] = self.name_
    result[fields.ImageSelector_ArgcOffset] = self.argc_
    return result


class Method(Object):
  
  def __init__(self, selector, lam):
    super(Method, self).__init__()
    self.selector_ = selector
    self.lam_ = lam
  
  def allocate(self, heap):
    size = fields.ImageMethod_Size
    result = heap.allocate(size, Smi(values.Method.index))
    result[fields.ImageMethod_SelectorOffset] = self.selector_
    result[fields.ImageMethod_SignatureOffset] = EMPTY_SIGNATURE
    result[fields.ImageMethod_LambdaOffset] = self.lam_
    return result


class Root(Object):
  
  def __init__(self, name):
    super(Root, self).__init__()
    self.name_ = name


VOID = Root('VoidValue')
NULL = Root('NullValue')
TRUE = Root('TrueValue')
FALSE = Root('FalseValue')


class Lambda(Object):
  
  def __init__(self, argc, body):
    super(Lambda, self).__init__()
    self.argc_ = argc
    self.body_ = body
  
  def allocate(self, heap):
    result = heap.allocate(fields.ImageLambda_Size, Smi(values.Lambda.index))
    result[fields.ImageLambda_ArgcOffset] = image.Raw(self.argc_)
    result[fields.ImageLambda_TreeOffset] = self.body_
    return result


class Protocol(Object):
  
  def __init__(self, name, members):
    super(Protocol, self).__init__()
    self.name_ = name
    self.members_ = members
  
  def allocate(self, heap):
    result = heap.allocate(fields.ImageProtocol_Size, Smi(values.Protocol.index))
    result[fields.ImageProtocol_NameOffset] = self.name_
    result[fields.ImageProtocol_MethodsOffset] = Tuple(entries = self.members_)
    return result


class Dictionary(Object):
  
  def __init__(self):
    super(Dictionary, self).__init__()
    self.map_ = { }
  
  def map(self):
    return self.map_

  def __setitem__(self, key, value):
    self.map()[key] = value
  
  def allocate(self, heap):
    length = len(self.map_)
    table = Tuple(length = length * 2)
    index = 0
    for (key, value) in self.map().items():
      table[index] = key
      table[index + 1] = value
      index += 2
    result = heap.allocate(fields.ImageDictionary_Size, Smi(values.Dictionary.index))
    result[fields.ImageDictionary_TableOffset] = table
    return result

  def __str__(self):
    items = ", ".join([ '%s: %s' % (k, v) for (k, v) in self.map().items() ])
    return "{%s}" % items


class SyntaxTree(Object):

  def __init__(self):
    super(SyntaxTree, self).__init__()


class Symbol(SyntaxTree):
  
  def __init__(self, name):
    super(Symbol, self).__init__()
    self.name_ = name


class Arguments(SyntaxTree):

  def __init__(self):
    super(Arguments, self).__init__()


class ReturnExpression(SyntaxTree):
  
  def __init__(self, value):
    super(ReturnExpression, self).__init__()
    self.value_ = value


class InternalCall(SyntaxTree):
  
  def __init__(self, argc, index):
    super(InternalCall, self).__init__()
    self.argc_ = argc
    self.index_ = index


class NativeCall(SyntaxTree):
  
  def __init__(self, argc, name):
    super(NativeCall, self).__init__()
    self.argc_ = argc
    self.name_ = name


class CallExpression(SyntaxTree):
  
  def __init__(self, recv, fun, args):
    super(CallExpression, self).__init__()
    self.recv_ = recv
    self.fun_ = fun
    self.args_ = args


class InvokeExpression(SyntaxTree):
  
  def __init__(self, recv, name, args):
    super(InvokeExpression, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args


class InstantiateExpression(SyntaxTree):
  
  def __init__(self, recv, name, args, terms):
    super(InstantiateExpression, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args
    self.terms_ = terms


class ThisExpression(SyntaxTree):
  
  def __init__(self):
    super(ThisExpression, self).__init__()


class GlobalExpression(SyntaxTree):
  
  def __init__(self, name):
    super(GlobalExpression, self).__init__()
    self.name_ = name


class LocalExpression(SyntaxTree):
  
  def __init__(self, symbol):
    super(LocalExpression, self).__init__()
    self.symbol_ = symbol


class LiteralExpression(SyntaxTree):
  
  def __init__(self, value):
    super(LiteralExpression, self).__init__()
    self.value_ = value


class SequenceExpression(SyntaxTree):
  
  def __init__(self, exprs):
    super(SequenceExpression, self).__init__()
    self.exprs_ = exprs


class ConditionalExpression(SyntaxTree):
  
  def __init__(self, cond, then_part, else_part):
    super(ConditionalExpression, self).__init__()
    self.cond_ = cond
    self.then_part_ = then_part
    self.else_part_ = else_part


class LocalDefinition(SyntaxTree):
  
  def __init__(self, name, value, body):
    super(LocalDefinition, self).__init__()
    self.name_ = name
    self.value_ = value
    self.body_ = body
