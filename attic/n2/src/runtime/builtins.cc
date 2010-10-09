#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "runtime/builtins-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"
#include "utils/globals.h"
#include "utils/string-inl.pp.h"
#include "values/values-inl.pp.h"

#include <ctype.h>

namespace neutrino {

// -----------------------------------
// --- I n f r a s t r u c t u r e ---
// -----------------------------------

builtin *Builtins::get(uword index) {
  switch (index) {

#define MAKE_CASE(n, type, name, str) case n: return &Builtins::type##_##name;
eBuiltinMethods(MAKE_CASE)
#undef MAKE_CASE

#define MAKE_CASE(n, name, str) case n: return &Builtins::name;
ePlainBuiltinFunctions(MAKE_CASE)
#undef MAKE_CASE

    default:
      UNREACHABLE();
      return NULL;
  }
}

// -------------------
// --- S t r i n g ---
// -------------------

Data *Builtins::string_length(BuiltinArguments &args) {
  @assert 0 == args.count();
  @check String *self = to<String>(args.self());
  return Smi::from_int(self->length());
}

Data *Builtins::string_hash(BuiltinArguments &args) {
  @assert 0 == args.count();
  @check String *self = to<String>(args.self());
  // TODO: This hash is exceedingly stupid.  Replace with something
  //   that gives a better distribution.
  uword value = 0;
  for (uword i = 0; i < self->length(); i++)
    value = (3 * value) + self->get(i);
  return Smi::from_int(value);
}

Data *Builtins::string_eq(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check String *self = to<String>(args.self());
  @check Immediate *other = to<Immediate>(args[0]);
  if (!is<String>(other)) return args.runtime().roots().fahlse();
  String *that = cast<String>(other);
  uword length = self->length();
  if (length != that->length())
    return args.runtime().roots().fahlse();
  for (uword i = 0; i < length; i++) {
    if (self->get(i) != that->get(i))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}

Data *Builtins::string_plus(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check String *self = to<String>(args.self());
  @check String *that = to<String>(args[0]);
  uword length = self->length() + that->length();
  @alloc String *result = args.runtime().heap().new_string(length);
  for (uword i = 0; i < self->length(); i++)
    result->set(i, self->get(i));
  for (uword i = 0; i < that->length(); i++)
    result->set(self->length() + i, that->get(i));
  return result;
}

Data *Builtins::string_get(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check String *self = to<String>(args.self());
  @check Smi *index = to<Smi>(args[0]);
  char c = self->get(index->value());
  return args.runtime().heap().new_string(string(&c, 1)).data();
}

Data *Builtins::is_whitespace(BuiltinArguments &args) {
  @assert 0 == args.count();
  @check String *self = to<String>(args.self());
  for (uword i = 0; i < self->length(); i++) {
    if (!isspace(self->get(i)))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}

Data *Builtins::is_alpha(BuiltinArguments &args) {
  @assert 0 == args.count();
  @check String *self = to<String>(args.self());
  for (uword i = 0; i < self->length(); i++) {
    if (!isalpha(self->get(i)))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}

Data *Builtins::is_digit(BuiltinArguments &args) {
  @assert 0 == args.count();
  @check String *self = to<String>(args.self());
  for (uword i = 0; i < self->length(); i++) {
    if (!isdigit(self->get(i)))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

Data *Builtins::smi_plus(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Smi *self = to<Smi>(args.self());
  @check Smi *that = to<Smi>(args[0]);
  return Smi::from_int(self->value() + that->value());
}

Data *Builtins::smi_minus(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Smi *self = to<Smi>(args.self());
  @check Smi *that = to<Smi>(args[0]);
  return Smi::from_int(self->value() - that->value());
}

Data *Builtins::smi_times(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Smi *self = to<Smi>(args.self());
  @check Smi *that = to<Smi>(args[0]);
  return Smi::from_int(self->value() * that->value());
}

Data *Builtins::smi_divide(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Smi *self = to<Smi>(args.self());
  @check Smi *that = to<Smi>(args[0]);
  return Smi::from_int(self->value() / that->value());
}

Data *Builtins::smi_modulo(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Smi *self = to<Smi>(args.self());
  @check Smi *that = to<Smi>(args[0]);
  return Smi::from_int(self->value() % that->value());
}

Data *Builtins::smi_less(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Smi *self = to<Smi>(args.self());
  @check Smi *that = to<Smi>(args[0]);
  return (self->value() < that->value())
    ? static_cast<Bool*>(args.runtime().roots().thrue())
    : static_cast<Bool*>(args.runtime().roots().fahlse());
}

Data *Builtins::smi_abs(BuiltinArguments &args) {
  @assert 0 == args.count();
  @check Smi *self = to<Smi>(args.self());
  word value = self->value();
  if (value >= 0) return self;
  else return Smi::from_int(-value);
}


// -------------------
// --- O b j e c t ---
// -------------------

Data *Builtins::object_eq(BuiltinArguments &args) {
  @assert 1 == args.count();
  Immediate *self = deref(args.self());
  Immediate *other = deref(args[0]);
  return (self == other)
       ? static_cast<Value*>(args.runtime().roots().thrue())
       : static_cast<Value*>(args.runtime().roots().fahlse());
}

Data *Builtins::object_to_string(BuiltinArguments &args) {
  @assert 0 == args.count();
  scoped_string str(args.self()->to_string());
  return args.runtime().heap().new_string(*str).data();
}


// ---------------------------------------
// --- C l a s s   E x p r e s s i o n ---
// ---------------------------------------

Data *Builtins::protocol_expression_evaluate(BuiltinArguments &args) {
  @assert 0 == args.count();
  ref_block<> protect(args.runtime().refs());
  @check ProtocolExpression *raw_expr = to<ProtocolExpression>(args.self());
  ref<ProtocolExpression> expr = protect(raw_expr);
  ref<Context> context = protect(args.lambda()->context());
  return expr.compile(args.runtime(), context).data();
}


// -----------------
// --- C l a s s ---
// -----------------

Data *Builtins::protocol_new(BuiltinArguments &args) {
  @assert 0 == args.count();
  Runtime &runtime = args.runtime();
  @check Protocol *protocol = to<Protocol>(args.self());
  if (protocol == runtime.roots().symbol_layout()->protocol()) {
    return runtime.heap().new_symbol(runtime.roots().vhoid()).data();
  } else {
    @alloc InstanceLayout *layout = runtime.heap().new_instance_layout(0, protocol, runtime.roots().empty_tuple());
    return runtime.heap().new_instance(layout).data();
  }
}


// -----------------
// --- T u p l e ---
// -----------------

Data *Builtins::tuple_eq(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Tuple *self = to<Tuple>(args.self());
  maybe<Tuple> other_opt = to<Tuple>(args[0]);
  if (other_opt.has_failed()) return args.runtime().roots().fahlse();
  Tuple *other = other_opt.value();
  if (self == other) return args.runtime().roots().thrue();
  if (is<Nothing>(other))
    return args.runtime().roots().fahlse();
  Tuple *that = cast<Tuple>(other);
  if (self->length() != that->length())
    return args.runtime().roots().fahlse();
  for (uword i = 0; i < self->length(); i++) {
    if (!self->get(i)->equals(that->get(i)))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}

Data *Builtins::tuple_get(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Tuple *self = to<Tuple>(args.self());
  @check Smi *index = to<Smi>(args[0]);
  return self->get(index->value());
}

Data *Builtins::tuple_length(BuiltinArguments &args) {
  @assert 0 == args.count();
  @check Tuple *self = to<Tuple>(args.self());
  return Smi::from_int(self->length());
}


// -----------------
// --- A r r a y ---
// -----------------

Data *Builtins::array_set(BuiltinArguments &args) {
  @assert 2 == args.count();
  @check Array *self = to<Array>(args.self());
  @check Smi *index = to<Smi>(args[0]);
  Value *value = args[1];
  self->set(index->value(), value);
  return value;
}

Data *Builtins::array_get(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Array *self = to<Array>(args.self());
  @check Smi *index = to<Smi>(args[0]);
  return self->get(index->value());
}

Data *Builtins::new_array(BuiltinArguments &args) {
  @assert 1 == args.count();
  @check Smi *size = to<Smi>(args[0]);
  return args.runtime().heap().new_array(size->value()).data();
}

Data *Builtins::array_length(BuiltinArguments &args) {
  @assert 0 == args.count();
  @check Array *self = to<Array>(args.self());
  return Smi::from_int(self->length());
}


// -------------------
// --- L a m b d a ---
// -------------------

Data *Builtins::lambda_disassemble(BuiltinArguments &args) {
  ref_block<> protect(args.runtime().refs());
  ref<Lambda> self = protect(cast<Lambda>(args.self()));
  self.ensure_compiled(args.runtime(), ref<Method>());
  string_buffer buf;
  args.runtime().architecture().disassemble(*self, buf);
  buf.raw_string().println();
  return args.runtime().roots().vhoid();
}


// -----------------------------------------
// --- L a m b d a   E x p r e s s i o n ---
// -----------------------------------------


Data *Builtins::lambda_expression_params(BuiltinArguments &args) {
  return cast<LambdaExpression>(args.self())->parameters()->parameters();
}


Data *Builtins::lambda_expression_body(BuiltinArguments &args) {
  return cast<LambdaExpression>(args.self())->body();
}


// -------------------------
// --- F u n c t i o n s ---
// -------------------------

Data *Builtins::raw_print(BuiltinArguments &args) {
  @assert args.count() == 1;
  @check String *str_obj = to<String>(args[0]);
  for (uword i = 0; i < str_obj->length(); i++)
    putc(str_obj->get(i), stdout);
  putc('\n', stdout);
  return args.runtime().roots().vhoid();
}

Data *Builtins::compile_expression(BuiltinArguments &args) {
  ref_block<> protect(args.runtime().refs());
  @check SyntaxTree *raw_self = to<SyntaxTree>(args.self());
  ref<SyntaxTree> self = protect(raw_self);
  ref<Context> context = protect(args.lambda()->context());
  @check(probably) ref<Lambda> code = Compiler::compile(args.runtime(), self, context);
  return *code;
}

Data *Builtins::lift(BuiltinArguments &args) {
  Value *value = args[0];
  return args.runtime().heap().new_literal_expression(value).data();
}

Data *Builtins::process_unquote(BuiltinArguments &args) {
  Immediate *self = deref(args.self());
  if (is<Symbol>(self)) {
    return args.runtime().heap().new_local_variable(cast<Symbol>(self)).data();
  } else {
    return self;
  }
}


// ---------------
// --- T e s t ---
// ---------------

Data *Builtins::new_forwarder(BuiltinArguments &args) {
  @assert args.count() == 0;
  return args.runtime().heap().new_forwarder(Forwarder::fwOpen, args.self()).data();
}

Data *Builtins::set_target(BuiltinArguments &args) {
  @assert args.count() == 1;
  Forwarder *forwarder = cast<Forwarder>(args.self());
  forwarder->descriptor()->set_target(args[0]);
  return args.runtime().roots().vhoid();
}

Data *Builtins::close(BuiltinArguments &args) {
  @assert args.count() == 0;
  Forwarder *forwarder = cast<Forwarder>(args.self());
  forwarder->descriptor()->set_type(Forwarder::fwClosed);
  return args.runtime().roots().vhoid();
}


// ---------------------------------------
// --- A s t   C o n s t r u c t i o n ---
// ---------------------------------------

#define FETCH_ARG(Type, __name__, Name, arg)                         \
    @check Type *__name__ = to<Type>(args[__offset__++]);

#define SET_FIELD(Type, __name__, Name, arg)                         \
  __result__->set_##__name__(__name__);

#define MAKE_AST_CONSTRUCTOR(Type, __type__)                         \
Data *Builtins::__type__##_new(BuiltinArguments &args) {             \
  int __offset__ = 0;                                                \
  use(__offset__);                                                   \
  e##Type##Fields(FETCH_ARG, 0)                                      \
  allocation<Type> __value__ = args.runtime().heap().allocate_##__type__(); \
  if (__value__.has_failed()) return __value__.signal();             \
  Type *__result__ = __value__.value();                              \
  e##Type##Fields(SET_FIELD, 0)                                      \
  return __result__;                                                 \
}

MAKE_AST_CONSTRUCTOR(InvokeExpression, invoke_expression)
MAKE_AST_CONSTRUCTOR(Selector, selector)
MAKE_AST_CONSTRUCTOR(LiteralExpression, literal_expression)
MAKE_AST_CONSTRUCTOR(Arguments, arguments)
MAKE_AST_CONSTRUCTOR(ProtocolExpression, protocol_expression)
MAKE_AST_CONSTRUCTOR(Parameters, parameters)
MAKE_AST_CONSTRUCTOR(ReturnExpression, return_expression)
MAKE_AST_CONSTRUCTOR(LambdaExpression, lambda_expression)
MAKE_AST_CONSTRUCTOR(MethodExpression, method_expression)
MAKE_AST_CONSTRUCTOR(SequenceExpression, sequence_expression)
MAKE_AST_CONSTRUCTOR(TupleExpression, tuple_expression)
MAKE_AST_CONSTRUCTOR(GlobalVariable, global_variable)
MAKE_AST_CONSTRUCTOR(CallExpression, call_expression)
MAKE_AST_CONSTRUCTOR(LocalVariable, local_variable)
MAKE_AST_CONSTRUCTOR(ConditionalExpression, conditional_expression)
MAKE_AST_CONSTRUCTOR(ThisExpression, this_expression)

#undef FETCH_ARG
#undef SET_FIELD
#undef MAKE_AST_CONSTRUCTOR


} // namespace neutrino