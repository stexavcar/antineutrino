#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "runtime/builtins-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"
#include "utils/globals.h"
#include "values/values-inl.h"

#include <ctype.h>

namespace neutrino {

// -----------------------------------
// --- I n f r a s t r u c t u r e ---
// -----------------------------------

builtin *Builtins::get(uword index) {
  switch (index) {

#define MAKE_CASE(n, type, name, str) case n: return &Builtins::type##_##name;
FOR_EACH_BUILTIN_METHOD(MAKE_CASE)
#undef MAKE_CASE

#define MAKE_CASE(n, name, str) case n: return &Builtins::name;
FOR_EACH_PLAIN_BUILTIN_FUNCTION(MAKE_CASE)
#undef MAKE_CASE

    default:
      UNREACHABLE();
      return NULL;
  }
}


#define SIGNAL_CHECK(Type, name, operation)                          \
  Data *name##_val = operation;                                      \
  if (is<Signal>(name##_val)) return name##_val;                     \
  Type *name = cast<Type>(name##_val);


// -------------------
// --- S t r i n g ---
// -------------------

Data *Builtins::string_length(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  SIGNAL_CHECK(String, self, to<String>(args.self()));
  return Smi::from_int(self->length());
}

Data *Builtins::string_eq(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(String, self, to<String>(args.self()));
  Data *other = to<String>(args[0]);
  if (is<Nothing>(other)) return args.runtime().roots().fahlse();
  String *that = cast<String>(other);
  uword length = self->length();
  if (length != that->length())
    return args.runtime().roots().fahlse();
  for (uword i = 0; i < length; i++) {
    if (self->at(i) != that->at(i))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}

Data *Builtins::string_plus(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(String, self, to<String>(args.self()));
  SIGNAL_CHECK(String, that, to<String>(args[0]));
  uword length = self->length() + that->length();
  String *result = cast<String>(Runtime::current().heap().new_string(length));
  for (uword i = 0; i < self->length(); i++)
    result->at(i) = self->at(i);
  for (uword i = 0; i < that->length(); i++)
    result->at(self->length() + i) = that->at(i);
  return result;
}

Data *Builtins::string_get(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(String, self, to<String>(args.self()));
  SIGNAL_CHECK(Smi, index, to<Smi>(args[0]));
  char c = self->at(index->value());
  return args.runtime().heap().new_string(string(&c, 1));
}

Data *Builtins::is_whitespace(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  SIGNAL_CHECK(String, self, to<String>(args.self()));
  for (uword i = 0; i < self->length(); i++) {
    if (!isspace(self->at(i)))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}

Data *Builtins::is_alpha(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  SIGNAL_CHECK(String, self, to<String>(args.self()));
  for (uword i = 0; i < self->length(); i++) {
    if (!isalpha(self->at(i)))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}

Data *Builtins::is_digit(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  SIGNAL_CHECK(String, self, to<String>(args.self()));
  for (uword i = 0; i < self->length(); i++) {
    if (!isdigit(self->at(i)))
      return args.runtime().roots().fahlse();
  }
  return args.runtime().roots().thrue();
}


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

Data *Builtins::smi_plus(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(Smi, self, to<Smi>(args.self()));
  SIGNAL_CHECK(Smi, that, to<Smi>(args[0]));
  return Smi::from_int(self->value() + that->value());
}

Data *Builtins::smi_minus(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(Smi, self, to<Smi>(args.self()));
  SIGNAL_CHECK(Smi, that, to<Smi>(args[0]));
  return Smi::from_int(self->value() - that->value());
}

Data *Builtins::smi_times(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(Smi, self, to<Smi>(args.self()));
  SIGNAL_CHECK(Smi, that, to<Smi>(args[0]));
  return Smi::from_int(self->value() * that->value());
}

Data *Builtins::smi_divide(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(Smi, self, to<Smi>(args.self()));
  SIGNAL_CHECK(Smi, that, to<Smi>(args[0]));
  return Smi::from_int(self->value() / that->value());
}

Data *Builtins::smi_less(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(Smi, self, to<Smi>(args.self()));
  SIGNAL_CHECK(Smi, that, to<Smi>(args[0]));
  return (self->value() < that->value())
    ? static_cast<Bool*>(args.runtime().roots().thrue())
    : static_cast<Bool*>(args.runtime().roots().fahlse());
}

Data *Builtins::smi_abs(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  SIGNAL_CHECK(Smi, self, to<Smi>(args.self()));
  word value = self->value();
  if (value >= 0) return self;
  else return Smi::from_int(-value);
}


// -------------------
// --- O b j e c t ---
// -------------------

Data *Builtins::object_eq(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  Immediate *self = deref(args.self());
  Immediate *other = deref(args[0]);
  return (self == other)
       ? static_cast<Value*>(Runtime::current().roots().thrue())
       : static_cast<Value*>(Runtime::current().roots().fahlse());
}

Data *Builtins::object_to_string(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  scoped_string str(args.self()->to_string());
  return Runtime::current().heap().new_string(*str);
}


// ---------------------------------------
// --- C l a s s   E x p r e s s i o n ---
// ---------------------------------------

Data *Builtins::protocol_expression_evaluate(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  SIGNAL_CHECK(ProtocolExpression, raw_expr, to<ProtocolExpression>(args.self()));
  RefScope scope;
  ref<ProtocolExpression> expr = new_ref(raw_expr);
  ref<Context> context = new_ref(args.lambda()->context());
  ref<Protocol> result = expr.compile(context);
  return *result;
}


// -----------------
// --- C l a s s ---
// -----------------

Data *Builtins::protocol_new(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  RefScope scope;
  Runtime &runtime = Runtime::current();
  SIGNAL_CHECK(Protocol, protocol, to<Protocol>(args.self()));
  if (protocol == runtime.roots().symbol_layout()->protocol()) {
    return runtime.heap().new_symbol(runtime.roots().vhoid());
  } else {
    Data *layout_val = runtime.heap().new_layout(tInstance, 0, protocol, runtime.roots().empty_tuple());
    if (is<AllocationFailed>(layout_val)) return layout_val;
    Layout *layout = cast<Layout>(layout_val);
    return runtime.heap().new_instance(layout);
  }
}


// -----------------
// --- T u p l e ---
// -----------------

Data *Builtins::tuple_eq(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(Tuple, self, to<Tuple>(args.self()));
  Data *other = to<Tuple>(args[0]);
  if (is<Nothing>(other))
    return Runtime::current().roots().fahlse();
  Tuple *that = cast<Tuple>(other);
  if (self->length() != that->length())
    return Runtime::current().roots().fahlse();
  for (uword i = 0; i < self->length(); i++) {
    if (!self->get(i)->equals(that->get(i)))
      return Runtime::current().roots().fahlse();
  }
  return Runtime::current().roots().thrue();
}

Data *Builtins::tuple_get(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(Tuple, self, to<Tuple>(args.self()));
  SIGNAL_CHECK(Smi, index, to<Smi>(args[0]));
  return self->get(index->value());
}

Data *Builtins::tuple_length(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  SIGNAL_CHECK(Tuple, self, to<Tuple>(args.self()));
  return Smi::from_int(self->length());
}


// -------------------
// --- L a m b d a ---
// -------------------

Data *Builtins::lambda_disassemble(BuiltinArguments &args) {
  ref<Lambda> self = new_ref(cast<Lambda>(args.self()));
  self.ensure_compiled();
  scoped_string str(self->disassemble());
  str->println();
  return Runtime::current().roots().vhoid();
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
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(String, str_obj, to<String>(args[0]));
  for (uword i = 0; i < str_obj->length(); i++)
    putc(str_obj->at(i), stdout);
  putc('\n', stdout);
  return Runtime::current().roots().vhoid();
}

Data *Builtins::compile_expression(BuiltinArguments &args) {
  SIGNAL_CHECK(SyntaxTree, raw_self, to<SyntaxTree>(args.self()));
  ref<SyntaxTree> self = new_ref(raw_self);
  ref<Context> context = new_ref(args.lambda()->context());
  ref<Lambda> code = Compiler::compile(self, context);
  return *code;
}

Data *Builtins::lift(BuiltinArguments &args) {
  Value *value = args[0];
  return Runtime::current().heap().new_literal_expression(value);
}


// ---------------------
// --- C h a n n e l ---
// ---------------------

Data *Builtins::channel_send(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  SIGNAL_CHECK(Channel, self, to<Channel>(args.self()));
  SIGNAL_CHECK(Immediate, message, to<Immediate>(args[0]));
  return self->send(args.runtime(), message);
}


// ---------------
// --- T e s t ---
// ---------------

Data *Builtins::make_forwarder(BuiltinArguments &args) {
  ASSERT_EQ(0, args.count());
  return args.runtime().heap().new_transparent_forwarder(args.self());
}

Data *Builtins::set_target(BuiltinArguments &args) {
  ASSERT_EQ(1, args.count());
  Forwarder *forwarder = cast<Forwarder>(args.self());
  forwarder->descriptor()->set_target(args[0]);
  return args.runtime().roots().vhoid();
}

} // namespace neutrino
