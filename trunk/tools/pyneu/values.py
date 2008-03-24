import codecs
from consts import values, fields, roots
import image
from image import Raw

class Value(object):
  pass


class Smi(Value):

  def __init__(self, value):
    super(Smi, self).__init__()
    self.value_ = value
  
  def __repr__(self):
    return "smi %i" % self.value_

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
      self.set_cache(heap, obj)
    return self.addresses[heap].tag_as_object()

  def set_cache(self, heap, obj):
    self.addresses[heap] = obj


class String(Object):

  def __init__(self, value):
    super(String, self).__init__()
    assert not value is None
    assert not isinstance(value, String)
    self.value_ = value

  def __str__(self):
    return '"%s"' % self.value_

  def allocate(self, heap):
    (chars, _) = codecs.utf_8_encode(self.value_)
    length = len(chars)
    size = fields.ImageString_HeaderSize + length
    result = heap.allocate(size, Smi(values.String.index))
    result[fields.ImageString_LengthOffset] = Raw(length)
    for i in xrange(length):
      char = chars[i]
      result[fields.ImageString_HeaderSize + i] = Raw(ord(char))
    return result


def to_tuple(elms):
  if elms is None: return NULL
  else: return Tuple(entries = elms)


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

  def __init__(self, elements):
    super(Signature, self).__init__()
    self.elements_ = elements

  def allocate(self, heap):
    result = heap.allocate(fields.ImageSignature_Size, Smi(values.Signature.index))
    result[fields.ImageSignature_ParametersOffset] = Tuple(entries = self.elements_)
    return result


EMPTY_SIGNATURE = Signature([])


def to_bool(self):
  if self: return TRUE
  else: return FALSE


class Selector(Object):

  def __init__(self, name, argc, keywords, is_accessor):
    super(Selector, self).__init__()
    assert not isinstance(name, String)
    assert not isinstance(argc, Smi)
    self.name_ = name
    self.argc_ = argc
    self.keywords_ = sorted(keywords)
    self.is_accessor_ = is_accessor

  def allocate(self, heap):
    result = heap.allocate(fields.ImageSelector_Size, Smi(values.Selector.index))
    result[fields.ImageSelector_NameOffset] = String(self.name_)
    result[fields.ImageSelector_ArgcOffset] = Smi(self.argc_)
    keywords = [ String(w) for w in self.keywords_ ]
    result[fields.ImageSelector_KeywordsOffset] = Tuple(entries = keywords)
    result[fields.ImageSelector_IsAccessorOffset] = to_bool(self.is_accessor_)
    return result


class Method(Object):

  def __init__(self, selector, lam, is_static):
    super(Method, self).__init__()
    self.selector_ = selector
    self.lam_ = lam
    self.is_static_ = is_static
    self.signature_ = None

  def set_signature(self, protocol):
    self.signature_ = Signature([protocol])

  def is_static(self):
    return self.is_static_

  def allocate(self, heap):
    size = fields.ImageMethod_Size
    result = heap.allocate(size, Smi(values.Method.index))
    result[fields.ImageMethod_SelectorOffset] = self.selector_
    result[fields.ImageMethod_SignatureOffset] = self.signature_
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
    result[fields.ImageLambda_ContextOffset] = Context()
    return result


class Protocol(Object):

  def __init__(self, name, methods, static_members, sup=None, is_static=False):
    super(Protocol, self).__init__()
    self.name_ = name
    self.methods_ = methods
    self.super_ = sup
    if is_static:
      self.static_protocol_ = None
    else:
      self.static_protocol_ = Protocol(None, static_members, [], VOID, True)

  def methods(self):
    return self.methods_

  def static_protocol(self):
    return self.static_protocol_
  
  def set_super(self, value):
    self.super_ = value

  def allocate(self, heap):
    if self.static_protocol_:
      layout = Layout(values.Protocol.index, self.static_protocol_)
    else:
      layout = Smi(values.Protocol.index)
    result = heap.allocate(fields.ImageProtocol_Size, layout)
    self.set_cache(heap, result)
    if self.name_:
      result[fields.ImageProtocol_NameOffset] = String(self.name_)
    result[fields.ImageProtocol_MethodsOffset] = Tuple(entries = self.methods_)
    result[fields.ImageProtocol_SuperOffset] = self.super_
    return result


class Layout(Object):

  def __init__(self, instance_type, protocol):
    super(Layout, self).__init__()
    self.instance_type_ = instance_type
    self.protocol_ = protocol

  def allocate(self, heap):
    result = heap.allocate(fields.ImageLayout_Size, Smi(values.Layout.index))
    result[fields.ImageLayout_InstanceTypeOffset] = Raw(self.instance_type_)
    result[fields.ImageLayout_ProtocolOffset] = self.protocol_
    result[fields.ImageLayout_MethodsOffset] = EMPTY_TUPLE
    return result


class Dictionary(Object):

  def __init__(self):
    super(Dictionary, self).__init__()
    self.map_ = { }

  def map(self):
    return self.map_

  def __setitem__(self, key, value):
    self.map()[key] = value

  def __getitem__(self, key):
    return self.map()[key]

  def allocate(self, heap):
    length = len(self.map_)
    table = Tuple(length = length * 2)
    index = 0
    for (key, value) in self.map().items():
      table[index] = String(key)
      table[index + 1] = value
      index += 2
    result = heap.allocate(fields.ImageDictionary_Size, Smi(values.Dictionary.index))
    result[fields.ImageDictionary_TableOffset] = table
    return result

  def __str__(self):
    items = ", ".join([ '%s: %s' % (k, v) for (k, v) in self.map().items() ])
    return "{%s}" % items


class Context(Object):

  def __init__(self):
    super(Context, self).__init__()

  def allocate(self, heap):
    result = heap.allocate(fields.ImageContext_Size, Smi(values.Context.index))
    return result


class SyntaxTree(Object):

  def __init__(self):
    super(SyntaxTree, self).__init__()


class LambdaExpression(SyntaxTree):

  def __init__(self, params, body):
    super(LambdaExpression, self).__init__()
    self.params_ = params
    self.body_ = body

  def allocate(self, heap):
    result = heap.allocate(fields.ImageLambdaExpression_Size, Smi(values.LambdaExpression.index))
    result[fields.ImageLambdaExpression_ParametersOffset] = self.params_
    result[fields.ImageLambdaExpression_BodyOffset] = self.body_
    return result


class TaskExpression(SyntaxTree):

  def __init__(self, body):
    super(TaskExpression, self).__init__()
    self.body_ = body

  def allocate(self, heap):
    result = heap.allocate(fields.ImageTaskExpression_Size, Smi(values.TaskExpression.index))
    result[fields.ImageTaskExpression_LambdaOffset] = self.body_
    return result


class MethodExpression(SyntaxTree):

  def __init__(self, selector, body, is_static):
    super(MethodExpression, self).__init__()
    self.selector_ = selector
    self.body_ = body
    self.is_static_ = is_static

  def allocate(self, heap):
    result = heap.allocate(fields.ImageMethodExpression_Size, Smi(values.MethodExpression.index))
    result[fields.ImageMethodExpression_SelectorOffset] = self.selector_
    result[fields.ImageMethodExpression_LambdaOffset] = self.body_
    result[fields.ImageMethodExpression_IsStaticOffset] = self.is_static_
    return result


class ProtocolExpression(SyntaxTree):

  def __init__(self, name, methods, shuper):
    super(ProtocolExpression, self).__init__()
    self.name_ = name
    self.methods_ = methods
    self.super_ = shuper

  def allocate(self, heap):
    result = heap.allocate(fields.ImageProtocolExpression_Size, Smi(values.ProtocolExpression.index))
    result[fields.ImageProtocolExpression_NameOffset] = String(self.name_)
    result[fields.ImageProtocolExpression_MethodsOffset] = Tuple(entries = self.methods_)
    result[fields.ImageProtocolExpression_SuperOffset] = self.super_
    return result


class QuoteExpression(SyntaxTree):

  def __init__(self, value, unquotes):
    super(QuoteExpression, self).__init__()
    self.value_ = value
    self.unquotes_ = unquotes

  def allocate(self, heap):
    result = heap.allocate(fields.ImageQuoteExpression_Size, Smi(values.QuoteExpression.index))
    result[fields.ImageQuoteExpression_ValueOffset] = self.value_
    result[fields.ImageQuoteExpression_UnquotesOffset] = Tuple(entries = self.unquotes_)
    return result


class UnquoteExpression(SyntaxTree):

  def __init__(self, index):
    super(UnquoteExpression, self).__init__()
    self.index_ = index

  def allocate(self, heap):
    result = heap.allocate(fields.ImageUnquoteExpression_Size, Smi(values.UnquoteExpression.index))
    result[fields.ImageUnquoteExpression_IndexOffset] = Raw(self.index_)
    return result


class Symbol(SyntaxTree):

  def __init__(self, name):
    super(Symbol, self).__init__()
    self.name_ = name

  def allocate(self, heap):
    result = heap.allocate(fields.ImageSymbol_Size, Smi(values.Symbol.index))
    result[fields.ImageSymbol_NameOffset] = String(self.name_)
    return result


class Arguments(SyntaxTree):

  def __init__(self, args, keywords):
    super(Arguments, self).__init__()
    self.args_ = args
    self.keywords_ = keywords

  def allocate(self, heap):
    result = heap.allocate(fields.ImageArguments_Size, Smi(values.Arguments.index))
    result[fields.ImageArguments_ArgumentsOffset] = Tuple(entries = self.args_)
    posc = len(self.args_) - len(self.keywords_)
    keyword_indices = [ ]
    for key in sorted(self.keywords_.keys()):
      keyword_indices.append(Smi(self.keywords_[key] - posc))
    result[fields.ImageArguments_KeywordIndicesOffset] = Tuple(entries = keyword_indices)
    return result


class Parameters(SyntaxTree):

  def __init__(self, posc, params):
    super(Parameters, self).__init__()
    self.posc_ = posc
    self.params_ = params
  
  def allocate(self, heap):
    result = heap.allocate(fields.ImageParameters_Size, Smi(values.Parameters.index))
    result[fields.ImageParameters_PositionCountOffset] = Smi(self.posc_)
    result[fields.ImageParameters_ParametersOffset] = Tuple(entries = self.params_)
    return result


class ReturnExpression(SyntaxTree):

  def __init__(self, value):
    super(ReturnExpression, self).__init__()
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.ImageReturnExpression_Size, Smi(values.ReturnExpression.index))
    result[fields.ImageReturnExpression_ValueOffset] = self.value_
    return result


class YieldExpression(SyntaxTree):

  def __init__(self, value):
    super(YieldExpression, self).__init__()
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.ImageYieldExpression_Size, Smi(values.YieldExpression.index))
    result[fields.ImageYieldExpression_ValueOffset] = self.value_
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
    result[fields.ImageExternalCall_ArgcOffset] = Smi(self.argc_)
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
    terms = [ ]
    for (k, v) in self.terms_.items():
      terms.append(String(k))
      terms.append(v)
    result[fields.ImageInstantiateExpression_TermsOffset] = Tuple(entries = terms)
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


class LiteralExpression(SyntaxTree):

  def __init__(self, value):
    super(LiteralExpression, self).__init__()
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.ImageLiteralExpression_Size, Smi(values.LiteralExpression.index))
    result[fields.ImageLiteralExpression_ValueOffset] = self.value_
    return result


class InterpolateExpression(SyntaxTree):

  def __init__(self, terms):
    super(InterpolateExpression, self).__init__()
    self.terms_ = terms

  def allocate(self, heap):
    result = heap.allocate(fields.ImageInterpolateExpression_Size, Smi(values.InterpolateExpression.index))
    result[fields.ImageInterpolateExpression_TermsOffset] = Tuple(entries = self.terms_)
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


class TupleExpression(SyntaxTree):

  def __init__(self, values):
    super(TupleExpression, self).__init__()
    self.values_ = values

  def allocate(self, heap):
    result = heap.allocate(fields.ImageTupleExpression_Size, Smi(values.TupleExpression.index))
    result[fields.ImageTupleExpression_ValuesOffset] = Tuple(entries = self.values_)
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


class Assignment(SyntaxTree):

  def __init__(self, symbol, value):
    super(Assignment, self).__init__()
    assert isinstance(symbol, Symbol)
    self.symbol_ = symbol
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.ImageAssignment_Size, Smi(values.Assignment.index))
    result[fields.ImageAssignment_SymbolOffset] = self.symbol_
    result[fields.ImageAssignment_ValueOffset] = self.value_
    return result


class DoOnExpression(SyntaxTree):

  def __init__(self, value, clauses):
    super(DoOnExpression, self).__init__()
    self.value_ = value
    self.clauses_ = clauses

  def allocate(self, heap):
    result = heap.allocate(fields.ImageDoOnExpression_Size, Smi(values.DoOnExpression.index))
    result[fields.ImageDoOnExpression_ValueOffset] = self.value_
    result[fields.ImageDoOnExpression_ClausesOffset] = Tuple(entries = self.clauses_)
    return result


class OnClause(SyntaxTree):

  def __init__(self, name, body):
    super(OnClause, self).__init__()
    self.name_ = name
    self.body_ = body

  def allocate(self, heap):
    result = heap.allocate(fields.ImageOnClause_Size, Smi(values.OnClause.index))
    result[fields.ImageOnClause_NameOffset] = String(self.name_)
    result[fields.ImageOnClause_LambdaOffset] = self.body_
    return result
