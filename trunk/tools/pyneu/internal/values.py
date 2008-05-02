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

  def get_int_value(self, anchor, heap):
    return (self.value_ << 2)


class Object(Value):

  def __init__(self):
    super(Object, self).__init__()
    self.addresses = { }

  def get_int_value(self, anchor, heap):
    if not heap in self.addresses:
      obj = self.allocate(heap)
      assert isinstance(obj, image.Address)
      self.set_cache(heap, obj)
    return self.addresses[heap].tag_as_object(anchor)

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
    size = fields.FString_HeaderSize + length
    result = heap.allocate(size, Smi(values.String.index))
    result[fields.FString_LengthOffset] = Raw(length)
    for i in xrange(length):
      char = chars[i]
      result[fields.FString_HeaderSize + i] = Raw(ord(char))
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
    size = fields.FTuple_HeaderSize + len(self.entries_)
    result = heap.allocate(size, Smi(values.Tuple.index))
    result[fields.FTuple_LengthOffset] = Raw(len(self.entries_))
    for i in xrange(len(self.entries_)):
      result[fields.FTuple_HeaderSize + i] = self.entries_[i]
    return result


EMPTY_TUPLE = Tuple(length = 0)


class Signature(Object):

  def __init__(self, elements):
    super(Signature, self).__init__()
    self.elements_ = elements

  def allocate(self, heap):
    result = heap.allocate(fields.FSignature_Size, Smi(values.Signature.index))
    result[fields.FSignature_ParametersOffset] = Tuple(entries = self.elements_)
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
    result = heap.allocate(fields.FSelector_Size, Smi(values.Selector.index))
    result[fields.FSelector_NameOffset] = String(self.name_)
    result[fields.FSelector_ArgcOffset] = Smi(self.argc_)
    keywords = [ String(w) for w in self.keywords_ ]
    result[fields.FSelector_KeywordsOffset] = Tuple(entries = keywords)
    result[fields.FSelector_IsAccessorOffset] = to_bool(self.is_accessor_)
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
    size = fields.FMethod_Size
    result = heap.allocate(size, Smi(values.Method.index))
    result[fields.FMethod_SelectorOffset] = self.selector_
    result[fields.FMethod_SignatureOffset] = self.signature_
    result[fields.FMethod_LambdaOffset] = self.lam_
    return result


class Root(Object):

  def __init__(self, name):
    super(Root, self).__init__()
    self.name_ = name

  def allocate(self, heap):
    index = roots[self.name_].index
    result = heap.allocate(fields.FRoot_Size, Smi(values.Root.index))
    result[fields.FRoot_IndexOffset] = Raw(index)
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
    result = heap.allocate(fields.FLambda_Size, Smi(values.Lambda.index))
    result[fields.FLambda_ArgcOffset] = Raw(self.argc_)
    result[fields.FLambda_TreeOffset] = self.body_
    result[fields.FLambda_ContextOffset] = Context()
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
    result = heap.allocate(fields.FProtocol_Size, layout)
    self.set_cache(heap, result)
    if self.name_:
      result[fields.FProtocol_NameOffset] = String(self.name_)
    result[fields.FProtocol_MethodsOffset] = Tuple(entries = self.methods_)
    result[fields.FProtocol_SuperOffset] = self.super_
    return result


class Layout(Object):

  def __init__(self, instance_type, protocol):
    super(Layout, self).__init__()
    self.instance_type_ = instance_type
    self.protocol_ = protocol

  def allocate(self, heap):
    result = heap.allocate(fields.FLayout_Size, Smi(values.Layout.index))
    result[fields.FLayout_InstanceTypeOffset] = Raw(self.instance_type_)
    result[fields.FLayout_ProtocolOffset] = self.protocol_
    result[fields.FLayout_MethodsOffset] = EMPTY_TUPLE
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
    for (key, value) in sorted(self.map().items()):
      table[index] = String(key)
      table[index + 1] = value
      index += 2
    result = heap.allocate(fields.FDictionary_Size, Smi(values.Dictionary.index))
    result[fields.FDictionary_TableOffset] = table
    return result

  def __str__(self):
    items = ", ".join([ '%s: %s' % (k, v) for (k, v) in self.map().items() ])
    return "{%s}" % items


class Channel(Object):

  def __init__(self, name):
    super(Channel, self).__init__()
    self.name_ = name
  
  def allocate(self, heap):
    result = heap.allocate(fields.FChannel_Size, Smi(values.Channel.index))
    result[fields.FChannel_NameOffset] = String(self.name_)
    return result


class Context(Object):

  def __init__(self):
    super(Context, self).__init__()

  def allocate(self, heap):
    result = heap.allocate(fields.FContext_Size, Smi(values.Context.index))
    return result


class SyntaxTree(Object):

  def __init__(self):
    super(SyntaxTree, self).__init__()


class LambdaExpression(SyntaxTree):

  def __init__(self, params, body, is_local):
    super(LambdaExpression, self).__init__()
    self.params_ = params
    self.body_ = body
    self.is_local_ = is_local

  def allocate(self, heap):
    result = heap.allocate(fields.FLambdaExpression_Size, Smi(values.LambdaExpression.index))
    result[fields.FLambdaExpression_ParametersOffset] = self.params_
    result[fields.FLambdaExpression_BodyOffset] = self.body_
    if self.is_local_: is_local_val = TRUE
    else: is_local_val = FALSE
    result[fields.FLambdaExpression_IsLocalOffset] = is_local_val
    return result


class TaskExpression(SyntaxTree):

  def __init__(self, body):
    super(TaskExpression, self).__init__()
    self.body_ = body

  def allocate(self, heap):
    result = heap.allocate(fields.FTaskExpression_Size, Smi(values.TaskExpression.index))
    result[fields.FTaskExpression_LambdaOffset] = self.body_
    return result


class MethodExpression(SyntaxTree):

  def __init__(self, selector, body, is_static):
    super(MethodExpression, self).__init__()
    self.selector_ = selector
    self.body_ = body
    self.is_static_ = is_static

  def allocate(self, heap):
    result = heap.allocate(fields.FMethodExpression_Size, Smi(values.MethodExpression.index))
    result[fields.FMethodExpression_SelectorOffset] = self.selector_
    result[fields.FMethodExpression_LambdaOffset] = self.body_
    result[fields.FMethodExpression_IsStaticOffset] = self.is_static_
    return result


class ProtocolExpression(SyntaxTree):

  def __init__(self, name, methods, shuper):
    super(ProtocolExpression, self).__init__()
    self.name_ = name
    self.methods_ = methods
    self.super_ = shuper

  def allocate(self, heap):
    result = heap.allocate(fields.FProtocolExpression_Size, Smi(values.ProtocolExpression.index))
    result[fields.FProtocolExpression_NameOffset] = String(self.name_)
    result[fields.FProtocolExpression_MethodsOffset] = Tuple(entries = self.methods_)
    result[fields.FProtocolExpression_SuperOffset] = self.super_
    return result


class QuoteExpression(SyntaxTree):

  def __init__(self, value, unquotes):
    super(QuoteExpression, self).__init__()
    self.value_ = value
    self.unquotes_ = unquotes

  def allocate(self, heap):
    result = heap.allocate(fields.FQuoteExpression_Size, Smi(values.QuoteExpression.index))
    result[fields.FQuoteExpression_ValueOffset] = self.value_
    result[fields.FQuoteExpression_UnquotesOffset] = Tuple(entries = self.unquotes_)
    return result


class UnquoteExpression(SyntaxTree):

  def __init__(self, index):
    super(UnquoteExpression, self).__init__()
    self.index_ = index

  def allocate(self, heap):
    result = heap.allocate(fields.FUnquoteExpression_Size, Smi(values.UnquoteExpression.index))
    result[fields.FUnquoteExpression_IndexOffset] = Raw(self.index_)
    return result


class Symbol(Object):

  def __init__(self, name):
    super(Symbol, self).__init__()
    self.name_ = name

  def allocate(self, heap):
    result = heap.allocate(fields.FSymbol_Size, Smi(values.Symbol.index))
    result[fields.FSymbol_NameOffset] = String(self.name_)
    return result


class Arguments(SyntaxTree):

  def __init__(self, args, keywords):
    super(Arguments, self).__init__()
    self.args_ = args
    self.keywords_ = keywords

  def allocate(self, heap):
    result = heap.allocate(fields.FArguments_Size, Smi(values.Arguments.index))
    result[fields.FArguments_ArgumentsOffset] = Tuple(entries = self.args_)
    posc = len(self.args_) - len(self.keywords_)
    keyword_indices = [ ]
    for key in sorted(self.keywords_.keys()):
      keyword_indices.append(Smi(self.keywords_[key] - posc))
    result[fields.FArguments_KeywordIndicesOffset] = Tuple(entries = keyword_indices)
    return result


class Parameters(SyntaxTree):

  def __init__(self, posc, params):
    super(Parameters, self).__init__()
    self.posc_ = posc
    self.params_ = params
  
  def allocate(self, heap):
    result = heap.allocate(fields.FParameters_Size, Smi(values.Parameters.index))
    result[fields.FParameters_PositionCountOffset] = Smi(self.posc_)
    result[fields.FParameters_ParametersOffset] = Tuple(entries = self.params_)
    return result


class ReturnExpression(SyntaxTree):

  def __init__(self, value):
    super(ReturnExpression, self).__init__()
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.FReturnExpression_Size, Smi(values.ReturnExpression.index))
    result[fields.FReturnExpression_ValueOffset] = self.value_
    return result


class YieldExpression(SyntaxTree):

  def __init__(self, value):
    super(YieldExpression, self).__init__()
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.FYieldExpression_Size, Smi(values.YieldExpression.index))
    result[fields.FYieldExpression_ValueOffset] = self.value_
    return result


class InternalCall(SyntaxTree):

  def __init__(self, argc, index):
    super(InternalCall, self).__init__()
    self.argc_ = argc
    self.index_ = index

  def allocate(self, heap):
    result = heap.allocate(fields.FBuiltinCall_Size, Smi(values.BuiltinCall.index))
    result[fields.FBuiltinCall_ArgcOffset] = Raw(self.argc_)
    result[fields.FBuiltinCall_IndexOffset] = Raw(self.index_)
    return result


class CallExpression(SyntaxTree):

  def __init__(self, recv, fun, args):
    super(CallExpression, self).__init__()
    self.recv_ = recv
    self.fun_ = fun
    self.args_ = args

  def allocate(self, heap):
    result = heap.allocate(fields.FCallExpression_Size, Smi(values.CallExpression.index))
    result[fields.FCallExpression_ReceiverOffset] = self.recv_
    result[fields.FCallExpression_FunctionOffset] = self.fun_
    result[fields.FCallExpression_ArgumentsOffset] = self.args_
    return result


class InvokeExpression(SyntaxTree):

  def __init__(self, recv, selector, args):
    super(InvokeExpression, self).__init__()
    self.recv_ = recv
    self.selector_ = selector
    self.args_ = args

  def allocate(self, heap):
    result = heap.allocate(fields.FInvokeExpression_Size, Smi(values.InvokeExpression.index))
    result[fields.FInvokeExpression_ReceiverOffset] = self.recv_
    result[fields.FInvokeExpression_SelectorOffset] = self.selector_
    result[fields.FInvokeExpression_ArgumentsOffset] = self.args_
    return result


class InstantiateExpression(SyntaxTree):

  def __init__(self, recv, name, args, terms):
    super(InstantiateExpression, self).__init__()
    self.recv_ = recv
    self.name_ = name
    self.args_ = args
    self.terms_ = terms

  def allocate(self, heap):
    result = heap.allocate(fields.FInstantiateExpression_Size, Smi(values.InstantiateExpression.index))
    result[fields.FInstantiateExpression_ReceiverOffset] = self.recv_
    result[fields.FInstantiateExpression_NameOffset] = String(self.name_)
    result[fields.FInstantiateExpression_ArgumentsOffset] = self.args_
    terms = [ ]
    for (k, v) in sorted(self.terms_.items()):
      terms.append(String(k))
      terms.append(v)
    result[fields.FInstantiateExpression_TermsOffset] = Tuple(entries = terms)
    return result


class RaiseExpression(SyntaxTree):

  def __init__(self, name, args):
    super(RaiseExpression, self).__init__()
    self.name_ = name
    self.args_ = args

  def allocate(self, heap):
    result = heap.allocate(fields.FRaiseExpression_Size, Smi(values.RaiseExpression.index))
    result[fields.FRaiseExpression_NameOffset] = String(self.name_)
    result[fields.FRaiseExpression_ArgumentsOffset] = self.args_
    return result


class ThisExpression(SyntaxTree):

  def __init__(self):
    super(ThisExpression, self).__init__()

  def allocate(self, heap):
    result = heap.allocate(fields.FThisExpression_Size, Smi(values.ThisExpression.index))
    return result


class GlobalVariable(SyntaxTree):

  def __init__(self, name):
    super(GlobalVariable, self).__init__()
    self.name_ = name

  def allocate(self, heap):
    result = heap.allocate(fields.FGlobalVariable_Size, Smi(values.GlobalVariable.index))
    result[fields.FGlobalVariable_NameOffset] = String(self.name_)
    return result


class LocalVariable(SyntaxTree):

  def __init__(self, symbol):
    super(LocalVariable, self).__init__()
    self.symbol_ = symbol

  def allocate(self, heap):
    result = heap.allocate(fields.FLocalVariable_Size, Smi(values.LocalVariable.index))
    result[fields.FLocalVariable_SymbolOffset] = self.symbol_
    return result


class SuperExpression(SyntaxTree):

  def __init__(self, value):
    super(SuperExpression, self).__init__()
    self.value_ = value
  
  def allocate(self, heap):
    result = heap.allocate(fields.FSuperExpression_Size, Smi(values.SuperExpression.index))
    result[fields.FSuperExpression_ValueOffset] = self.value_
    return result


class LiteralExpression(SyntaxTree):

  def __init__(self, value):
    super(LiteralExpression, self).__init__()
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.FLiteralExpression_Size, Smi(values.LiteralExpression.index))
    result[fields.FLiteralExpression_ValueOffset] = self.value_
    return result


class InterpolateExpression(SyntaxTree):

  def __init__(self, terms):
    super(InterpolateExpression, self).__init__()
    self.terms_ = terms

  def allocate(self, heap):
    result = heap.allocate(fields.FInterpolateExpression_Size, Smi(values.InterpolateExpression.index))
    result[fields.FInterpolateExpression_TermsOffset] = Tuple(entries = self.terms_)
    return result


class SequenceExpression(SyntaxTree):

  def __init__(self, exprs):
    super(SequenceExpression, self).__init__()
    self.exprs_ = exprs

  def allocate(self, heap):
    result = heap.allocate(fields.FSequenceExpression_Size, Smi(values.SequenceExpression.index))
    result[fields.FSequenceExpression_ExpressionsOffset] = Tuple(entries = self.exprs_)
    return result


class ConditionalExpression(SyntaxTree):

  def __init__(self, cond, then_part, else_part):
    super(ConditionalExpression, self).__init__()
    self.cond_ = cond
    self.then_part_ = then_part
    self.else_part_ = else_part

  def allocate(self, heap):
    result = heap.allocate(fields.FConditionalExpression_Size, Smi(values.ConditionalExpression.index))
    result[fields.FConditionalExpression_ConditionOffset] = self.cond_
    result[fields.FConditionalExpression_ThenPartOffset] = self.then_part_
    result[fields.FConditionalExpression_ElsePartOffset] = self.else_part_
    return result


class WhileExpression(SyntaxTree):

  def __init__(self, cond, body):
    super(WhileExpression, self).__init__()
    self.cond_ = cond
    self.body_ = body
  
  def allocate(self, heap):
    result = heap.allocate(fields.FWhileExpression_Size, Smi(values.WhileExpression.index))
    result[fields.FWhileExpression_ConditionOffset] = self.cond_
    result[fields.FWhileExpression_BodyOffset] = self.body_
    return result


class TupleExpression(SyntaxTree):

  def __init__(self, values):
    super(TupleExpression, self).__init__()
    self.values_ = values

  def allocate(self, heap):
    result = heap.allocate(fields.FTupleExpression_Size, Smi(values.TupleExpression.index))
    result[fields.FTupleExpression_ValuesOffset] = Tuple(entries = self.values_)
    return result


class LocalDefinition(SyntaxTree):

  def __init__(self, symbol, value, body, type):
    super(LocalDefinition, self).__init__()
    assert isinstance(symbol, Symbol)
    self.symbol_ = symbol
    self.value_ = value
    self.body_ = body
    self.type_ = type

  def allocate(self, heap):
    result = heap.allocate(fields.FLocalDefinition_Size, Smi(values.LocalDefinition.index))
    result[fields.FLocalDefinition_SymbolOffset] = self.symbol_
    result[fields.FLocalDefinition_ValueOffset] = self.value_
    result[fields.FLocalDefinition_BodyOffset] = self.body_
    result[fields.FLocalDefinition_TypeOffset] = Smi(self.type_)
    return result


class Assignment(SyntaxTree):

  def __init__(self, symbol, value):
    super(Assignment, self).__init__()
    assert isinstance(symbol, Symbol)
    self.symbol_ = symbol
    self.value_ = value

  def allocate(self, heap):
    result = heap.allocate(fields.FAssignment_Size, Smi(values.Assignment.index))
    result[fields.FAssignment_SymbolOffset] = self.symbol_
    result[fields.FAssignment_ValueOffset] = self.value_
    return result


class DoOnExpression(SyntaxTree):

  def __init__(self, value, clauses):
    super(DoOnExpression, self).__init__()
    self.value_ = value
    self.clauses_ = clauses

  def allocate(self, heap):
    result = heap.allocate(fields.FDoOnExpression_Size, Smi(values.DoOnExpression.index))
    result[fields.FDoOnExpression_ValueOffset] = self.value_
    result[fields.FDoOnExpression_ClausesOffset] = Tuple(entries = self.clauses_)
    return result


class OnClause(SyntaxTree):

  def __init__(self, name, body):
    super(OnClause, self).__init__()
    self.name_ = name
    self.body_ = body

  def allocate(self, heap):
    result = heap.allocate(fields.FOnClause_Size, Smi(values.OnClause.index))
    result[fields.FOnClause_NameOffset] = String(self.name_)
    result[fields.FOnClause_LambdaOffset] = self.body_
    return result
