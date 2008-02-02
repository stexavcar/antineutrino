#include "heap/ref-inl.h"
#include "io/image.h"
#include "runtime/runtime-inl.h"
#include "values/values-inl.h"

namespace neutrino {

Runtime *Runtime::current_ = NULL;

Runtime::Runtime()
  : heap_(roots_)
  , factory_(*this)
  , interpreter_(*this) {
}

bool Runtime::initialize() {
  if (!roots().initialize(heap())) return false;
  return true;
}

void Runtime::start() {
  Runtime::Scope runtime_scope(*this);
  Data *value = roots().toplevel()->get(cast<String>(heap().new_string("main")));
  if (is<Nothing>(value)) {
    Conditions::get().error_occurred("Error: no function 'main' was defined.");
  } else if (!is<Lambda>(value)) {
    Conditions::get().error_occurred("Value 'main' is not a function.");
  }
  Task *task = cast<Task>(heap().new_task());
  cast<Lambda>(value)->call(task);
}

bool Runtime::load_image(Image &image) {
  if (!image.initialize()) return false;
  Runtime::Scope runtime_scope(*this);
  Tuple *roots = image.load();
  RefScope ref_scope;
  return install_loaded_roots(new_ref(roots));
}

bool Runtime::install_loaded_roots(ref<Tuple> roots) {
  for (uword i = 0; i < roots.length(); i++) {
    RefScope scope;
    ref<Value> raw_changes = roots.get(i);
    if (is<Smi>(raw_changes)) continue;
    ref<Object> changes = cast<Object>(raw_changes);
    ref<Object> root = get_root(i);
    if (!install_object(root, changes)) return false;
  }
  return true;
}

bool Runtime::install_object(ref<Object> root, ref<Object> changes) {
  InstanceType type = root.type();
  switch (type) {
    case DICTIONARY_TYPE:
      return install_dictionary(cast<Dictionary>(root), cast<Dictionary>(changes));
    case LAYOUT_TYPE:
      return install_layout(cast<Layout>(root), cast<Protocol>(changes));
    default:
      UNHANDLED(InstanceType, type);
      return false;
  }
}

bool Runtime::install_dictionary(ref<Dictionary> root, ref<Dictionary> changes) {
  // First copy all elements into the tables so that we can iterate
  // through the elements independent of whether a gc occurs or not
  uword length = changes.size();
  ref<Tuple> keys = factory().new_tuple(length);
  ref<Tuple> values = factory().new_tuple(length);
  Dictionary::Iterator iter(*changes);
  Dictionary::Iterator::Entry entry;
  for (uword i = 0; i < length; i++) {
    bool next_result = iter.next(&entry);
    USE(next_result); ASSERT(next_result);
    keys->set(i, entry.key);
    values->set(i, entry.value);
  }
  // Check that we've reached the end
  ASSERT(!iter.next(&entry));
  // Then add the elements to the root object
  for (uword i = 0; i < length; i++) {
    RefScope scope;
    root.set(keys.get(i), values.get(i));
  }
  return true;
}

bool Runtime::install_layout(ref<Layout> root, ref<Protocol> changes) {
  if (!root->is_empty()) {
    scoped_string str(changes.name().to_string());
    Conditions::get().error_occurred("Root class %s is not empty.", str.chars());
    return false;
  }
  root->set_protocol(*changes);
  ASSERT(!root->is_empty());
  return true;
}

}
