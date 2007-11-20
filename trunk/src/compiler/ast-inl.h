#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "compiler/ast.h"
#include "heap/values-inl.h"

namespace neutrino {

DEFINE_FIELD_ACCESSORS(Value, LiteralExpression, value, kValueOffset)

DEFINE_FIELD_ACCESSORS(SyntaxTree, InvokeExpression, receiver, kReceiverOffset)
DEFINE_FIELD_ACCESSORS(String, InvokeExpression, name, kNameOffset)
DEFINE_FIELD_ACCESSORS(Tuple, InvokeExpression, arguments, kArgumentsOffset)

DEFINE_FIELD_ACCESSORS(SyntaxTree, CallExpression, receiver, kReceiverOffset)
DEFINE_FIELD_ACCESSORS(SyntaxTree, CallExpression, function, kFunctionOffset)
DEFINE_FIELD_ACCESSORS(Tuple, CallExpression, arguments, kArgumentsOffset)

DEFINE_FIELD_ACCESSORS(String, ClassExpression, name, kNameOffset)
DEFINE_FIELD_ACCESSORS(Tuple, ClassExpression, methods, kMethodsOffset)
DEFINE_FIELD_ACCESSORS(Value, ClassExpression, super, kSuperOffset)

DEFINE_FIELD_ACCESSORS(SyntaxTree, ReturnExpression, value, kValueOffset)

DEFINE_FIELD_ACCESSORS(String, MethodExpression, name, kNameOffset)
DEFINE_FIELD_ACCESSORS(SyntaxTree, MethodExpression, body, kBodyOffset)

DEFINE_FIELD_ACCESSORS(Tuple, SequenceExpression, expressions, kExpressionsOffset)

DEFINE_FIELD_ACCESSORS(Tuple, TupleExpression, values, kValuesOffset)

DEFINE_FIELD_ACCESSORS(String, GlobalExpression, name, kNameOffset)

}

#endif // _IO_AST_INL
