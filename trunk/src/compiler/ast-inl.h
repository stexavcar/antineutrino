#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "compiler/ast.h"
#include "heap/values-inl.h"

namespace neutrino {

FOR_EACH_QUOTE_TEMPLATE_FIELD(DEFINE_FIELD_ACCESSORS, QuoteTemplate)

DEFINE_ACCESSORS(uint32_t, BuiltinCall, argc, Argc)
DEFINE_ACCESSORS(uint32_t, BuiltinCall, index, Index)
DEFINE_ACCESSORS(uint32_t, UnquoteExpression, index, Index)

QuoteTemplateScope::QuoteTemplateScope(Visitor &visitor,
    ref<QuoteTemplate> value)
    : visitor_(visitor)
    , value_(value)
    , previous_(visitor.quote_scope()) {
  visitor.quote_scope_ = this;
}

QuoteTemplateScope::~QuoteTemplateScope() {
  visitor_.quote_scope_ = previous_;
}

}

#endif // _IO_AST_INL
