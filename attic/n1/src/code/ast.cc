#include "code/ast.h"
#include "io/read-inl.h"
#include "runtime/gc-safe-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/array-inl.h"
#include "value/value-inl.h"
#include "value/condition-inl.h"

namespace neutrino {

template <typename T>
T *CodePointer<T>::operator->() {
  return reinterpret_cast<T*>(stream().code().start() + offset());
}

template <typename T>
CodePointer<T> CodeStream::add(const T &obj) {
  word start = cursor();
  for (word i = 0; i < T::kHeaderSize; i++)
    code().append(obj.code()[i]);
  for (word i = T::kHeaderSize; i < obj.size(); i++)
    code().append(0);
  return CodePointer<T>(start, *this);
}

likely<SyntaxTree> CodeStream::result() {
  protector<1> protect(runtime().refs());
  try alloc ref<Blob> blob = runtime().gc_safe().new_blob(code().length() * sizeof(code_t));
  code().as_vector().copy_to(blob->as_vector<code_t>());
  return runtime().gc_safe().new_syntax_tree(blob, literals().store());
}

probably CodeGenerator::emit_expression(s_exp *expr) {
  if (is<s_number>(expr)) {
    // Numbers
    word value = cast<s_number>(expr)->value();
    code().add(literal(value));
  } else if (is<s_string>(expr)) {
    // Identifiers
    string str = cast<s_string>(expr)->chars();
    protector<1> protect(runtime().refs());
    try alloc ref<String> name = runtime().gc_safe().new_string(str);
    word index = 0;
    try probably code().register_literal(name, &index);
    code().add(global(index));
  } else {
    assert is<s_list>(expr);
    s_list *list = cast<s_list>(expr);
    assert (list->length() > 0);
    assert (is<s_string>(list->get(0)));
    string str = cast<s_string>(list->get(0))->chars();
    if (str == "!") {
      // Sends
      assert (list->length() > 2);
      string name = cast<s_string>(list->get(2))->chars();
      word receiver = code().cursor();
      emit_expression(list->get(1));
      word argc = list->length() - 3;
      buffer<code_t> args;
      for (word i = 3; i < list->length(); i++) {
        args.append(code().cursor());
        emit_expression(list->get(i));
      }
      protector<1> protect(runtime().refs());
      try alloc ref<String> name_str = runtime().gc_safe().new_string(name);
      word index = 0;
      try probably code().register_literal(name_str, &index);
      CodePointer<send> s = code().add(send(index, receiver, argc));
      for (word i = 0; i < argc; i++)
        s->args()[i] = args[i];
    } else if (str == "if") {
      assert list->length() == 4;
      word cond = code().cursor();
      emit_expression(list->get(1));
      CodePointer<if_false> goto_else = code().add(if_false(0));
      word then_part = code().cursor();
      emit_expression(list->get(2));
      CodePointer<ghoto> goto_end = code().add(ghoto(0));
      word else_part = code().cursor();
      goto_else->set_target(else_part);
      emit_expression(list->get(3));
      code().add(if_true_false(cond, then_part, else_part));
      goto_end->set_target(code().cursor());
    } else if (str == "fn") {
      assert list->length() == 3;
      CodeGenerator codegen(runtime());
      codegen.emit_expression(list->get(2));

    } else {
      return FatalError::abort();
    }
  }
  return Success::make();
}

probably CodeStream::register_literal(ref<Value> value, word *index) {
  *index = literals().length();
  return literals().add(runtime(), value);
}

probably ArrayBuffer::add(Runtime &runtime, ref<Value> value) {
  if (store().is_empty()) {
    protector<1> protect(runtime.refs());
    try alloc ref<Array> store = runtime.gc_safe().new_array(4);
    store_ = runtime.refs().new_persistent(*store);
  }
  if (cursor_ >= store()->length()) {
    protector<1> protect(runtime.refs());
    word new_size = grow_value(store()->length());
    try alloc ref<Array> new_store = runtime.gc_safe().new_array(new_size);
    for (word i = 0; i < cursor_; i++)
      new_store->set(i, store()->get(i));
    store().set(*new_store);
  }
  store()->set(cursor_++, *value);
  return Success::make();
}

} // namespace neutrino
