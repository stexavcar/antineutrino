from consts import values, fields, roots
import image
from image import Raw

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
    assert not isinstance(value, String)
    self.value_ = value

  def __str__(self):
    return '"%s"' % self.value_

  def allocate(self, heap):
    length = len(self.value_)
    size = fields.ImageString_HeaderSize + length
    result = heap.allocate(size, Smi(values.String.index))
    result[fields.ImageString_LengthOffset] = Raw(length)
    for i in xrange(length):
      char = self.value_[i]
      result[fields.ImageString_HeaderSize + i] = Raw(ord(char))
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
    result[fields.ImageTuple_LengthOffset] = Raw(len(self.entries_))
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
    assert not isinstance(name, String)
    assert not isinstance(argc, Smi)
    self.name_ = name
    self.argc_ = argc

  def allocate(self, heap):
    result = heap.allocate(fields.ImageSelector_Size, Smi(values.Selector.index))
    result[fields.ImageSelector_NameOffset] = String(self.name_)
    result[fields.ImageSelector_ArgcOffset] = Smi(self.argc_)
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

  def allocate(self, heap):
    index = roots[self.name_].index
    result = heap.allocate(fields.ImageRoot_Size, Smi(values.Singleton.index))
    result[fields.ImageRoot_IndexOffset] = Raw(index)
    return result


VOID = Root('vhoid')
NULL = Root('nuhll')
TRUE = Root('thrue')
FALSE = Root('fahlse')


class Lambda(Object):

  def __init__(self, argc, body):
    super(Lambda, self).__init__()
    self.argc_ = argc
    self.body_ = body

  def allocate(self, heap):
    result = heap.allocate(fields.ImageLambda_Size, Smi(values.Lambda.index))
    result[fields.ImageLambda_ArgcOffset] = Raw(self.argc_)
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

  def allocate(self, heap):
    result = heap.allocate(fields.ImageSymbol_Size, Smi(values.Symbol.index))
    result[fields.ImageSymbol_NameOffset] = String(self.name_)
    return result


class Arguments(SyntaxTree):

  def __init__(self):
    super(Arguments, self).__init__()

  def allocate(self, heap):
    result = heap.allocate(fields.ImageArguments_Size, Smi(values.Arguments.index))
    return result


class ReturnExpression(SyntaxTree):

  def __init__(self, value):
    super(ReturnExpression, self).__init__()
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.ImageReturnExpression_Size, Smi(values.ReturnExpression.index))
    result[fields.ImageReturnExpression_ValueOffset] = self.value_
    return result


class InternalCall(SyntaxTree):

  def __init__(self, argc, index):
    super(InternalCall, self).__init__()
    self.argc_ = argc
    self.index_ = index

  def allocate(self, heap):
    result = heap.allocate(fields.ImageBuiltinCall_Size, Smi(values.BuiltinCall.index))
    result[fields.ImageBuiltinCall_ArgcOffset] = Raw(self.argc_)
    result[fields.ImageBuiltinCall_IndexOffset] = Raw(self.index_)
    return result


class NativeCall(SyntaxTree):

  def __init__(self, argc, name):
    super(NativeCall, self).__init__()
    self.argc_ = argc
    self.name_ = name

  def allocate(self, heap):
    result = heap.allocate(fields.ImageExternalCall_Size, Smi(values.ExternalCall.index))
    result[fields.ImageExternalCall_ArgcOffset] = Raw(self.argc_)
    result[fields.ImageExternalCall_NameOffset] = String(self.name_)
    return result


class CallExpression(SyntaxTree):

  def __init__(self, recv, fun, args):
    super(CallExpression, self).__init__()
    self.recv_ = recv
    self.fun_ = fun
    self.args_ = args

  def allocate(self, heap):
    result = heap.allocate(fields.ImageCallExpression_Size, Smi(values.CallExpression.index))
    result[fields.ImageCallExpression_ReceiverOffset] = self.recv_
    result[fields.ImageCallExpression_FunctionOffset] = self.fun_
    result[fields.ImageCallExpression_ArgumentsOffset] = self.args_
    return result


class InvokeExpression(SyntaxTree):

  def __init__(self, recv, selector, args):
    super(InvokeExpression, self).__init__()
    self.recv_ = recv
    self.selector_ = selector
    self.args_ = args

  def allocate(self, heap):
    result = heap.allocate(fields.ImageInvokeExpression_Size, Smi(values.InvokeExpression.index))
    result[fields.ImageInvokeExpression_ReceiverOffset] = self.recv_
    result[fields.ImageInvokeExpression_SelectorOffset] = self.selector_
    result[fields.ImageInvokeExpression_ArgumentsOffset] = self.args_
    return result


class InstantiateExpression(SyntaxTree):

  def __init__(self, recv, name, args, terms):
    super(InstantiateExpression, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args
    self.terms_ = terms

  def allocate(self, heap):
    result = heap.allocate(fields.ImageInstantiateExpression_Size, Smi(values.InstantiateExpression.index))
    result[fields.ImageInstantiateExpression_ReceiverOffset] = self.recv_
    result[fields.ImageInstantiateExpression_NameOffset] = String(self.name_)
    result[fields.ImageInstantiateExpression_ArgumentsOffset] = self.args_
    return result


class RaiseExpression(SyntaxTree):

  def __init__(self, name, args):
    super(RaiseExpression, self).__init__()
    self.name_ = name
    self.args_ = args

  def allocate(self, heap):
    result = heap.allocate(fields.ImageRaiseExpression_Size, Smi(values.RaiseExpression.index))
    result[fields.ImageRaiseExpression_NameOffset] = String(self.name_)
    result[fields.ImageRaiseExpression_ArgumentsOffset] = self.args_
    return result


class ThisExpression(SyntaxTree):

  def __init__(self):
    super(ThisExpression, self).__init__()

  def allocate(self, heap):
    result = heap.allocate(fields.ImageThisExpression_Size, Smi(values.ThisExpression.index))
    return result


class GlobalExpression(SyntaxTree):

  def __init__(self, name):
    super(GlobalExpression, self).__init__()
    self.name_ = name

  def allocate(self, heap):
    result = heap.allocate(fields.ImageGlobalExpression_Size, Smi(values.GlobalExpression.index))
    result[fields.ImageGlobalExpression_NameOffset] = String(self.name_)
    return result


class LocalExpression(SyntaxTree):

  def __init__(self, symbol):
    super(LocalExpression, self).__init__()
    self.symbol_ = symbol


class LiteralExpression(SyntaxTree):

  def __init__(self, value):
    super(LiteralExpression, self).__init__()
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.ImageLiteralExpression_Size, Smi(values.LiteralExpression.index))
    return result


class SequenceExpression(SyntaxTree):

  def __init__(self, exprs):
    super(SequenceExpression, self).__init__()
    self.exprs_ = exprs

  def allocate(self, heap):
    result = heap.allocate(fields.ImageSequenceExpression_Size, Smi(values.SequenceExpression.index))
    result[fields.ImageSequenceExpression_ExpressionsOffset] = Tuple(entries = self.exprs_)
    return result


class ConditionalExpression(SyntaxTree):

  def __init__(self, cond, then_part, else_part):
    super(ConditionalExpression, self).__init__()
    self.cond_ = cond
    self.then_part_ = then_part
    self.else_part_ = else_part

  def allocate(self, heap):
    result = heap.allocate(fields.ImageConditionalExpression_Size, Smi(values.ConditionalExpression.index))
    result[fields.ImageConditionalExpression_ConditionOffset] = self.cond_
    result[fields.ImageConditionalExpression_ThenPartOffset] = self.then_part_
    result[fields.ImageConditionalExpression_ElsePartOffset] = self.else_part_
    return result


class LocalDefinition(SyntaxTree):

  def __init__(self, symbol, value, body):
    super(LocalDefinition, self).__init__()
    assert isinstance(symbol, Symbol)
    self.symbol_ = symbol
    self.value_ = value
    self.body_ = body

  def allocate(self, heap):
    result = heap.allocate(fields.ImageLocalDefinition_Size, Smi(values.LocalDefinition.index))
    result[fields.ImageLocalDefinition_SymbolOffset] = self.symbol_
    result[fields.ImageLocalDefinition_ValueOffset] = self.value_
    result[fields.ImageLocalDefinition_BodyOffset] = self.body_
    return result
