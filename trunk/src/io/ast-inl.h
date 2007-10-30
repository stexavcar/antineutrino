#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "heap/values-inl.h"
#include "io/ast.h"

namespace neutrino {

// --- L i t e r a l ---

DEFINE_ACCESSOR(Value*, Literal, value, kValueOffset)

}

#endif // _IO_AST_INL