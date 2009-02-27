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
  return reinterpret_cast<T*>(stream().buf().start() + offset());
}

template <typename T>
CodePointer<T> CodeStream::add_instr(const T &obj) {
  word start = cursor();
  for (word i = 0; i < T::kHeaderSize; i++)
    buf().append(obj.code()[i]);
  for (word i = T::kHeaderSize; i < obj.size(); i++)
    buf().append(0);
  return CodePointer<T>(start, *this);
}

likely<Array> CodeStream::literals() {
  return *literal_buffer().store();
}

probably CodeStream::add(s_exp *expr) {
  if (is<s_number>(expr)) {
    // Numbers
    word value = cast<s_number>(expr)->value();
    add_instr(literal(value));
  } else if (is<s_string>(expr)) {
    // Identifiers
    string str = cast<s_string>(expr)->chars();
    protector<1> protect(runtime().refs());
    try alloc ref<String> name = runtime().gc_safe().new_string(str);
    word index = 0;
    try probably register_literal(name, &index);
    add_instr(global(index));
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
      word receiver = cursor();
      add(list->get(1));
      word argc = list->length() - 3;
      buffer<code_t> args;
      for (word i = 3; i < list->length(); i++) {
        args.append(cursor());
        add(list->get(i));
      }
      protector<1> protect(runtime().refs());
      try alloc ref<String> name_str = runtime().gc_safe().new_string(name);
      word index = 0;
      try probably register_literal(name_str, &index);
      CodePointer<send> s = add_instr(send(index, receiver, argc));
      for (word i = 0; i < argc; i++)
        s->args()[i] = args[i];
    } else if (str == "if") {
      assert list->length() == 4;
      word cond = cursor();
      add(list->get(1));
      CodePointer<if_false> goto_else = add_instr(if_false(0));
      word then_part = cursor();
      add(list->get(2));
      CodePointer<ghoto> goto_end = add_instr(ghoto(0));
      word else_part = cursor();
      goto_else->set_target(else_part);
      add(list->get(3));
      add_instr(if_true_false(cond, then_part, else_part));
      goto_end->set_target(cursor());
    } else {
      assert false;
    }
  }
  return Success::make();
}

probably CodeStream::register_literal(ref<Value> value, word *index) {
  *index = literal_buffer().length();
  return literal_buffer().add(runtime(), value);
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
