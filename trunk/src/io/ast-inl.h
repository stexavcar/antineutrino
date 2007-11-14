#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "heap/values-inl.h"
#include "io/ast.h"

namespace neutrino {

// --- L i t e r a l ---

DEFINE_ACCESSOR(Value*, Literal, value, kValueOffset)

// --- I n v o k e ---

DEFINE_ACCESSORS(SyntaxTree*, Invoke, receiver, kReceiverOffset)
DEFINE_ACCESSORS(String*, Invoke, name, kNameOffset)
DEFINE_ACCESSORS(Tuple*, Invoke, arguments, kArgumentsOffset)


}

#endif // _IO_AST_INL