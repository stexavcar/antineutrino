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
  V(number, NUMBER)     V(class, CLASS)       V(builtin, BUILTIN)    \
  V(method, METHOD)

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

ref<Class> Parser::get_builtin_class(uint32_t index) {
  switch (index) {
    case 0:
      return runtime().true_class();
    case 1:
      return runtime().false_class();
    case 2:
      return runtime().string_class();
    case 3:
      return runtime().smi_class();
    default:
      UNREACHABLE();
      return 0;
  }
}

void Parser::load_definition(s::List &def) {
  string name = s::cast<s::String>(def[1]).str();
  ref<Value> value = parse_value(s::cast<s::List>(def[2]));
  runtime().toplevel().set(factory().new_string(name), value);
}

ref<Method> Parser::parse_method(s::List &ast) {
  ASSERT(ast.tag() == METHOD);
  string name_str = s::cast<s::String>(ast[1]).str();
  ref<String> name = runtime().factory().new_string(name_str);
  s::List &body = s::cast<s::List>(ast[2]);
  ASSERT(body.tag() == LAMBDA);
  int32_t argc = s::cast<s::Number>(body[1]).value();
  ref<Code> code = parse_code(s::cast<s::List>(body[2]));
  ref<Tuple> literals = parse_literals(s::cast<s::List>(body[3]));
  ref<Lambda> lambda = factory().new_lambda(argc, code, literals);
  return runtime().factory().new_method(name, lambda);
}

void Parser::load_builtin(s::List &decl) {
  uint32_t index = s::cast<s::Number>(decl[1]).value();
  ref<Class> type = get_builtin_class(index);
  s::List &method_asts = s::cast<s::List>(decl[2]);
  ref<Tuple> methods = runtime().factory().new_tuple(method_asts.length());
  for (uint32_t i = 0; i < method_asts.length(); i++) {
    s::List &method_ast = s::cast<s::List>(method_asts[i]);
    ref<Method> method = parse_method(method_ast);
    methods.set(i, method);
  }
  type->set_methods(*methods);
}

void Parser::load_declaration(s::List &decl) {
  switch (decl.tag()) {
    case DEFINE:
      load_definition(decl);
      break;
    case BUILTIN:
      load_builtin(decl);
      break;
    default:
      UNHANDLED(TreeTag, decl.tag());
  }
}

void Parser::load(sexp::List &program) {
  ASSERT_EQ(PROGRAM, program.tag());
  for (uint32_t i = 1; i < program.length(); i++) {
    RefScope ref_scope;
    sexp::List &decl = sexp::cast<sexp::List>(program[i]);
    load_declaration(decl);
  }
}

} // namespace neutrino
