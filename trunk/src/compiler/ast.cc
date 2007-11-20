#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

// -------------------------
// --- C o m p i l i n g ---
// -------------------------

ref<Class> ref_traits<ClassExpression>::compile() {
  ref<ClassExpression> self = open(this);
  Factory &factory = Runtime::current().factory();
  ref<Tuple> method_asts = methods();
  ref<Tuple> methods = factory.new_tuple(method_asts.length());
  for (uint32_t i = 0; i < method_asts.length(); i++) {
    RefScope scope;
    ref<MethodExpression> method_ast = cast<MethodExpression>(method_asts.get(i));
    ref<Method> method = method_ast.compile();
    methods.set(i, method);
  }
  ref<Class> result = factory.new_empty_class(INSTANCE_TYPE);
  result->set_name(self->name());
  result->set_methods(*methods);
  result->set_super(self->super());
  return result;
}

ref<Method> ref_traits<MethodExpression>::compile() {
  ref<MethodExpression> self = open(this);
  ref<Lambda> code = Compiler::compile(body());
  ref<Method> result = Runtime::current().factory().new_method(name(), code);
  return result;
}


// -------------------------
// --- U n p a r s i n g ---
// -------------------------

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


// ---------------------
// --- V i s i t o r ---
// ---------------------

void ref_traits<SyntaxTree>::accept(Visitor &visitor) {
  InstanceType type = this->type();
  ref<SyntaxTree> self = open(this);
  switch (type) {
  case LITERAL_EXPRESSION_TYPE:
    return visitor.visit_literal_expression(cast<LiteralExpression>(self));
  case RETURN_EXPRESSION_TYPE:
    return visitor.visit_return_expression(cast<ReturnExpression>(self));
  case SEQUENCE_EXPRESSION_TYPE:
    return visitor.visit_sequence_expression(cast<SequenceExpression>(self));
  default:
    UNHANDLED(InstanceType, type);
  }
}

void ref_traits<SyntaxTree>::traverse(Visitor &visitor) {
  InstanceType type = this->type();
  ref<SyntaxTree> self = open(this);
  switch (type) {
  case RETURN_EXPRESSION_TYPE:
    cast<ReturnExpression>(self).value().accept(visitor);
    break;
  case LITERAL_EXPRESSION_TYPE:
    break;
  case SEQUENCE_EXPRESSION_TYPE: {
    RefScope scope;
    ref<Tuple> expressions = cast<SequenceExpression>(self).expressions();
    for (uint32_t i = 0; i < expressions.length(); i++)
      cast<SyntaxTree>(expressions.get(i)).accept(visitor);
    break;
  }
  default:
    UNHANDLED(InstanceType, type);
  }
}

Visitor::~Visitor() { }

void Visitor::visit_node(ref<SyntaxTree> that) {
  that.traverse(*this);
}

void Visitor::visit_literal_expression(ref<LiteralExpression> that) {
  visit_node(that);
}

void Visitor::visit_return_expression(ref<ReturnExpression> that) {
  visit_node(that);
}

void Visitor::visit_sequence_expression(ref<SequenceExpression> that) {
  visit_node(that);
}

}
