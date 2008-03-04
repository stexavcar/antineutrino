#include "heap/ref-inl.h"
#include "io/image.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"
#include "values/values-inl.h"

namespace neutrino {

Runtime *Runtime::current_ = NULL;

Runtime::Runtime(DynamicLibraryCollection *dylibs)
  : heap_(roots_)
  , factory_(*this)
  , interpreter_(*this)
  , dylibs_(dylibs) {
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

void Runtime::report_load_error(ImageLoadInfo &info) {
  switch (info.status) {
    case ImageLoadInfo::TYPE_MISMATCH: {
      static const char *kErrorMessage =
        "Type mismatch while loading image\n"
        "  Expected: %s\n"
        "  Found: %s";
      const char *expected_str = Layout::layout_name(info.error_info.type_mismatch.expected);
      const char *found_str = Layout::layout_name(info.error_info.type_mismatch.found);
      Conditions::get().error_occurred(kErrorMessage, expected_str,
          found_str);
      return;
    }
    case ImageLoadInfo::INVALID_IMAGE: {
      static const char *kErrorMessage = "Invalid image";
      Conditions::get().error_occurred(kErrorMessage);
      return;
    }
    case ImageLoadInfo::ROOT_COUNT: {
      static const char *kErrorMessage = 
        "Invalid root count\n"
        "  Expected: %i\n"
        "  Found: %i";
      Conditions::get().error_occurred(kErrorMessage,
          info.error_info.root_count.expected,
          info.error_info.root_count.found);
      return;
    }
    case ImageLoadInfo::INVALID_MAGIC: {
      static const char *kErrorMessage =
        "Invalid image\n"
        "  Expected magic number: 0x%08X\n"
        "  Found: 0x%08X";
      Conditions::get().error_occurred(kErrorMessage,
          Image::kMagicNumber,
          info.error_info.magic.found);
      return;
    }
    case ImageLoadInfo::INVALID_VERSION: {
      static const char *kErrorMessage =
        "Invalid image\n"
        "  Expected version: %i\n"
        "  Found: %i";
      Conditions::get().error_occurred(kErrorMessage,
          Image::kCurrentVersion,
          info.error_info.version.found);
      return;
    }
    default:
      UNHANDLED(ImageLoadInfo::Status, info.status);
      break;
  }
}

bool Runtime::load_image(Image &image) {
  ImageLoadInfo info;
  image.initialize(info);
  if (info.has_error()) {
    report_load_error(info);
    return false;    
  }
  Runtime::Scope runtime_scope(*this);
  Data *roots_val = image.load(info);
  if (info.has_error()) {
    report_load_error(info);
    return false;
  } else {
    Tuple *roots = cast<Tuple>(roots_val);
    RefScope ref_scope;
    return install_loaded_roots(new_ref(roots));
  }
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
