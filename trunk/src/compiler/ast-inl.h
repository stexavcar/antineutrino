#ifndef _IO_AST_INL
#define _IO_AST_INL

#include "compiler/ast.h"
#include "values/values-inl.h"

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
  visitor.set_quote_scope(this);
}

QuoteTemplateScope::~QuoteTemplateScope() {
  visitor_.set_quote_scope(previous_);
}

ref<QuoteTemplate> Visitor::current_quote() {
  ASSERT(quote_scope() != NULL);
  return quote_scope()->value();
}

}

#endif // _IO_AST_INL
