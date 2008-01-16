#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "heap/values-inl.h"
#include "runtime/builtins-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"
#include "utils/globals.h"

namespace neutrino {


// -----------------------------------
// --- I n f r a s t r u c t u r e ---
// -----------------------------------

builtin *Builtins::get(uint32_t index) {
  switch (index) {

#define MAKE_CASE(n, type, name, str) case n: return &Builtins::type##_##name;
FOR_EACH_BUILTIN_METHOD(MAKE_CASE)
#undef MAKE_CASE

#define MAKE_CASE(n, name, str) case n: return &Builtins::name;
FOR_EACH_BUILTIN_FUNCTION(MAKE_CASE)
#undef MAKE_CASE

    default:
      UNREACHABLE();
      return NULL;
  }
}


// -------------------
// --- S t r i n g ---
// -------------------

Data *Builtins::string_length(Arguments &args) {
  ASSERT_EQ(0, args.count());
  String *self = cast<String>(args.self());
  return Smi::from_int(self->length());
}

Data *Builtins::string_eq(Arguments &args) {
  ASSERT_EQ(1, args.count());
  String *self = cast<String>(args.self());
  if (!is<String>(args[0])) return Runtime::current().roots().fahlse();
  String *that = cast<String>(args[0]);
  uint32_t length = self->length();
  if (length != that->length())
    return Runtime::current().roots().fahlse();
  for (uint32_t i = 0; i < length; i++) {
    if (self->at(i) != that->at(i))
      return Runtime::current().roots().fahlse();
  }
  return Runtime::current().roots().thrue();
}

Data *Builtins::string_plus(Arguments &args) {
  ASSERT_EQ(1, args.count());
  String *self = cast<String>(args.self());
  String *that = cast<String>(args[0]);
  uint32_t length = self->length() + that->length();
  String *result = cast<String>(Runtime::current().heap().new_string(length));
  for (uint32_t i = 0; i < self->length(); i++)
    result->at(i) = self->at(i);
  for (uint32_t i = 0; i < that->length(); i++)
    result->at(self->length() + i) = that->at(i);
  return result;
}


// ---------------------------------
// --- S m a l l   I n t e g e r ---
// ---------------------------------

Data *Builtins::smi_plus(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() + that->value());
}

Data *Builtins::smi_minus(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() - that->value());
}

Data *Builtins::smi_times(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() * that->value());
}

Data *Builtins::smi_divide(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Smi *self = cast<Smi>(args.self());
  Smi *that = cast<Smi>(args[0]);
  return Smi::from_int(self->value() / that->value());
}

Data *Builtins::smi_abs(Arguments &args) {
  ASSERT_EQ(0, args.count());
  Smi *self = cast<Smi>(args.self());
  int32_t value = self->value();
  if (value >= 0) return self;
  else return Smi::from_int(-value);
}


// -------------------
// --- O b j e c t ---
// -------------------

Data *Builtins::object_eq(Arguments &args) {
  ASSERT_EQ(1, args.count());
  return (args.self() == args[0])
       ? static_cast<Value*>(Runtime::current().roots().thrue())
       : static_cast<Value*>(Runtime::current().roots().fahlse());
}

Data *Builtins::object_to_string(Arguments &args) {
  ASSERT_EQ(0, args.count());
  scoped_string str(args.self()->to_string());
  return cast<Value>(Runtime::current().heap().new_string(*str));
}


// ---------------------------------------
// --- C l a s s   E x p r e s s i o n ---
// ---------------------------------------

Data *Builtins::layout_expression_evaluate(Arguments &args) {
  ASSERT_EQ(0, args.count());
  RefScope scope;
  ref<LayoutExpression> expr = new_ref(cast<LayoutExpression>(args.self()));
  ref<Context> context = new_ref(args.lambda()->context());
  ref<Layout> result = expr.compile(context);
  return *result;
}


// -----------------
// --- C l a s s ---
// -----------------

Data *Builtins::layout_new(Arguments &args) {
  ASSERT_EQ(0, args.count());
  RefScope scope;
  Runtime &runtime = Runtime::current();
  Layout *layout = cast<Layout>(args.self());
  InstanceType type = layout->instance_type();
  switch (type) {
    case INSTANCE_TYPE:
      return runtime.heap().new_instance(layout);
    case SYMBOL_TYPE:
      return runtime.heap().new_symbol(runtime.roots().vhoid());
    default:
      UNHANDLED(InstanceType, type);
      return 0;
  }
}


// -----------------
// --- T u p l e ---
// -----------------

Data *Builtins::tuple_eq(Arguments &args) {
  ASSERT_EQ(1, args.count());
  Tuple *self = cast<Tuple>(args.self());
  Value *other = args[0];
  if (!is<Tuple>(other))
    return Runtime::current().roots().fahlse();
  Tuple *that = cast<Tuple>(other);
  if (self->length() != that->length())
    return Runtime::current().roots().fahlse();
  for (uint32_t i = 0; i < self->length(); i++) {
    if (!self->get(i)->equals(that->get(i)))
      return Runtime::current().roots().fahlse();
  }
  return Runtime::current().roots().thrue();
}


// -------------------
// --- L a m b d a ---
// -------------------

Data *Builtins::lambda_disassemble(Arguments &args) {
  ref<Lambda> self = new_ref(cast<Lambda>(args.self()));
  self.ensure_compiled();
  scoped_string str(self->disassemble());
  str->println();
  return Runtime::current().roots().vhoid();
}


// -----------------------------------------
// --- L a m b d a   E x p r e s s i o n ---
// -----------------------------------------

Data *Builtins::lambda_expression_params(Arguments &args) {
  return cast<LambdaExpression>(args.self())->params();
}

Data *Builtins::lambda_expression_body(Arguments &args) {
  return cast<LambdaExpression>(args.self())->body();
}


// -------------------------
// --- F u n c t i o n s ---
// -------------------------

Data *Builtins::raw_print(Arguments &args) {
  ASSERT_EQ(1, args.count());
  String *str_obj = cast<String>(args[0]);
  for (uint32_t i = 0; i < str_obj->length(); i++)
    putc(str_obj->at(i), stdout);
  putc('\n', stdout);
  return Runtime::current().roots().vhoid();
}

Data *Builtins::compile_expression(Arguments &args) {
  ref<SyntaxTree> self = new_ref(cast<SyntaxTree>(args.self()));
  ref<Context> context = new_ref(args.lambda()->context());
  ref<Lambda> code = Compiler::compile(self, context);
  return *code;
}

Data *Builtins::lift(Arguments &args) {
  Value *value = args[0];
  return Runtime::current().heap().new_literal_expression(value);
}

} // namespace neutrino
