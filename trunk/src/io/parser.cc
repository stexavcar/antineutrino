#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "io/ast-inl.h"
#include "io/parser.h"
#include "utils/checks.h"

namespace neutrino {

namespace s = sexp;

// --- S y m b o l   R e s o l v e r ---

#define FOR_EACH_KIND(V)                                             \
  V(program, PROGRAM)   V(lambda, LAMBDA)     V(define,  DEFINE)     \
  V(number, NUMBER)

enum TreeTag {
  _FIRST_TREE_TAG
#define DEFINE_ENUM_CONST(name, NAME) , NAME
FOR_EACH_KIND(DEFINE_ENUM_CONST)
#undef DEFINE_ENUM_CONST
};

int ProgramSymbolResolver::get_kind_for(string str) {
#define MAKE_CASE(name, NAME) if (str == #name) return NAME;
FOR_EACH_KIND(MAKE_CASE)
#undef MAKE_CASE
  return -1;
}

string ProgramSymbolResolver::get_name_for(int32_t kind) {
  switch (kind) {
#define MAKE_CASE(name, NAME) case NAME: return #name;
FOR_EACH_KIND(MAKE_CASE)
#undef MAKE_CASE
  default: return "?";
  }
}

MAKE_ENUM_INFO_HEADER(TreeTag)
#define MAKE_CASE(name, NAME) MAKE_ENUM_INFO_ENTRY(NAME)
FOR_EACH_KIND(MAKE_CASE)
#undef MAKE_CASE
MAKE_ENUM_INFO_FOOTER()

// --- P a r s e r ---

Parser::Parser(Runtime &runtime)
    : runtime_(runtime)
    , factory_(runtime.factory()) {  
}

ref<SyntaxTree> Parser::parse_expression(s::List &expr) {
  switch (expr.tag()) {
  case NUMBER: {
    ref<Value> value = parse_value(expr);
    return factory().new_literal(value);
  }
  default:
    UNHANDLED(TreeTag, expr.tag());
    return ref<SyntaxTree>::empty();
  }
}

ref<Code> Parser::parse_code(s::List &expr) {
  uint32_t length = expr.length();
  ref<Code> result = runtime().factory().new_code(length);
  for (uint32_t i = 0; i < length; i++) {
    s::Number &value = s::cast<s::Number>(expr[i]);
    result.at(i) = value.value();
  }
  return result;
}

ref<Value> Parser::parse_value(s::Sexp &expr) {
  if (expr.type() == sexp::NUMBER) {
    s::Number &value = s::cast<s::Number>(expr);
    return new_ref(Smi::from_int(value.value()));
  } else if (expr.type() == sexp::STRING) {
    s::String &value = s::cast<s::String>(expr);
    return runtime().factory().new_string(value.str());
  } else {
    UNREACHABLE();
    return ref<Value>::empty();
  }
}

ref<Tuple> Parser::parse_literals(s::List &expr) {
  uint32_t length = expr.length();
  ref<Tuple> result = runtime().factory().new_tuple(length);
  for (uint32_t i = 0; i < length; i++) {
    ref<Value> value = parse_value(expr[i]);
    result->at(i) = *value;
  }
  return result;
}

ref<Value> Parser::parse_value(sexp::List &expr) {
  switch (expr.tag()) {
    case NUMBER: {
      int32_t value = s::cast<s::Number>(expr[1]).value();
      return new_ref(Smi::from_int(value));
    }
    case LAMBDA: {
      int32_t argc = s::cast<s::Number>(expr[1]).value();
      ref<Code> code = parse_code(s::cast<s::List>(expr[2]));
      ref<Tuple> literals = parse_literals(s::cast<s::List>(expr[3]));
      return factory().new_lambda(argc, code, literals);
    }
    default:
      UNREACHABLE();
      return ref<Value>::empty();
  }
}

void Parser::load(sexp::List &program) {
  ASSERT_EQ(PROGRAM, program.tag());
  for (uint32_t i = 1; i < program.length(); i++) {
    RefScope ref_scope;
    sexp::List &decl = sexp::cast<sexp::List>(program[i]);
    ASSERT_EQ(DEFINE, decl.tag());
    string name = sexp::cast<s::String>(decl[1]).str();
    ref<Value> value = parse_value(s::cast<s::List>(decl[2]));
    runtime().toplevel().set(factory().new_string(name), value);
  }
}

} // namespace neutrino
