#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "compiler/ast.h"
#include "heap/values-inl.h"

namespace neutrino {

// --- L i t e r a l   E x p r e s s i o n ---

DEFINE_ACCESSORS(Value*, LiteralExpression, value, kValueOffset)

// --- I n v o k e   E x p r e s s i o n ---

DEFINE_ACCESSORS(SyntaxTree*, InvokeExpression, receiver, kReceiverOffset)
DEFINE_ACCESSORS(String*, InvokeExpression, name, kNameOffset)
DEFINE_ACCESSORS(Tuple*, InvokeExpression, arguments, kArgumentsOffset)

// --- C l a s s   E x p r e s s i o n ---

DEFINE_FIELD_ACCESSORS(String, ClassExpression, name, kNameOffset)
DEFINE_FIELD_ACCESSORS(Tuple, ClassExpression, methods, kMethodsOffset)
DEFINE_FIELD_ACCESSORS(Class, ClassExpression, super, kSuperOffset)

// --- R e t u r n   E x p r e s s i o n ---

DEFINE_ACCESSORS(SyntaxTree*, ReturnExpression, value, kValueOffset)

// --- M e t h o d   E x p r e s s i o n ---

DEFINE_FIELD_ACCESSORS(String, MethodExpression, name, kNameOffset)
DEFINE_FIELD_ACCESSORS(SyntaxTree, MethodExpression, body, kBodyOffset)

}

#endif // _IO_AST_INL
