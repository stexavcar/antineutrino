#include "compiler/ast-inl.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

ref<Class> ref_traits<ClassExpression>::evaluate() {
  ref<ClassExpression> self = open(this);
  ref<Class> result = Runtime::current().factory().new_empty_class(OBJECT_TYPE);
  result->name() = self->name();
  return result;
}

static void unparse_literal_expression_on(LiteralExpression *obj,
    string_buffer &buf) {
  obj->value()->write_on(buf);
}

static void unparse_invoke_expression_on(InvokeExpression *obj, string_buffer &buf) {
  obj->receiver()->unparse_on(buf);
  buf.append('.');
  obj->name()->write_chars_on(buf);
  buf.append('(');
  bool is_first = true;
  for (uint32_t i = 0; i < obj->arguments()->length(); i++) {
    if (is_first) is_first = false;
    else buf.append(", ");
    cast<SyntaxTree>(obj->arguments()->at(i))->unparse_on(buf);
  }
  buf.append(')');
}

static void unparse_class_expression(ClassExpression *obj, string_buffer &buf) {
  buf.append("class ");
  obj->name()->write_chars_on(buf);
  buf.append(" {");
  Tuple *methods = obj->methods();
  for (uint32_t i = 0; i < methods->length(); i++) {
    MethodExpression *method = cast<MethodExpression>(methods->at(i));
    buf.append(" ");
    method->unparse_on(buf);
  }
  buf.append(" }");
}

static void unparse_return_expression(ReturnExpression *obj, string_buffer &buf) {
  buf.append("return ");
  obj->value()->unparse_on(buf);
}

static void unparse_method_expression(MethodExpression *obj, string_buffer &buf) {
  buf.append("def ");
  obj->name()->write_chars_on(buf);
  buf.append("() -> ");
  obj->body()->unparse_on(buf);
  buf.append(";");
}

static void unparse_syntax_tree_on(SyntaxTree *obj, string_buffer &buf) {
  InstanceType type = obj->type();
  switch (type) {
  case LITERAL_EXPRESSION_TYPE:
    unparse_literal_expression_on(cast<LiteralExpression>(obj), buf);
    break;
  case INVOKE_EXPRESSION_TYPE:
    unparse_invoke_expression_on(cast<InvokeExpression>(obj), buf);
    break;
  case CLASS_EXPRESSION_TYPE:
    unparse_class_expression(cast<ClassExpression>(obj), buf);
    break;
  case METHOD_EXPRESSION_TYPE:
    unparse_method_expression(cast<MethodExpression>(obj), buf);
    break;
  case RETURN_EXPRESSION_TYPE:
    unparse_return_expression(cast<ReturnExpression>(obj), buf);
    break;
  default:
    UNHANDLED(InstanceType, type);
  }
}

void SyntaxTree::unparse_on(string_buffer &buf) {
  unparse_syntax_tree_on(this, buf);
}

}
