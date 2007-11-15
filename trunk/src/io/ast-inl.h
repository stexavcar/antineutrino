#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "heap/values-inl.h"
#include "io/ast.h"

namespace neutrino {

// --- L i t e r a l   E x p r e s s i o n ---

DEFINE_ACCESSOR(Value*, LiteralExpression, value, kValueOffset)

// --- I n v o k e   E x p r e s s i o n ---

DEFINE_ACCESSORS(SyntaxTree*, InvokeExpression, receiver, kReceiverOffset)
DEFINE_ACCESSORS(String*, InvokeExpression, name, kNameOffset)
DEFINE_ACCESSORS(Tuple*, InvokeExpression, arguments, kArgumentsOffset)

// --- C l a s s   E x p r e s s i o n ---

DEFINE_ACCESSORS(String*, ClassExpression, name, kNameOffset)

}

#endif // _IO_AST_INL
