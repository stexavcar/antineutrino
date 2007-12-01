#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "compiler/ast.h"
#include "heap/values-inl.h"

namespace neutrino {

DEFINE_ACCESSORS(uint32_t, BuiltinCall, argc, Argc)
DEFINE_ACCESSORS(uint32_t, BuiltinCall, index, Index)

}

#endif // _IO_AST_INL
