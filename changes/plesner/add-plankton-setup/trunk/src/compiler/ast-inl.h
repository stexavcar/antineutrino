#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "compiler/ast.h"
#include "values/values-inl.h"

namespace neutrino {


DEFINE_ACCESSORS(uword, BuiltinCall, argc, Argc)
DEFINE_ACCESSORS(uword, BuiltinCall, index, Index)


bool Parameters::has_keywords() {
  return static_cast<uword>(position_count()->value()) < length();
}

uword Parameters::length() {
  return parameters()->length();
}

}

#endif // _IO_AST_INL
