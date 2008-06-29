#include "heap/ref-inl.h"
#include "io/image.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"
#include "values/values-inl.h"

namespace neutrino {

Runtime::Runtime(DynamicLibraryCollection *dylibs)
  : heap_(roots_)
  , factory_(*this)
  , gc_safe_(*this)
  , architecture_(NULL)
  , dylibs_(dylibs) {
}

Runtime::~Runtime() {
  if (architecture_ != NULL)
    architecture().dispose();
}

Signal *Runtime::initialize(Architecture *arch) {
  @try roots().initialize(heap());
  architecture_ = arch;
  if (arch == NULL) return Success::make();
  else return architecture().setup(*this);
}

Signal *Runtime::start() {
  ref_block<> protect(refs());
  @check ref<String> main_name = factory().new_string("entry_point");
  Data *entry_point = roots().toplevel()->get(*main_name);
  if (is<Nothing>(entry_point)) {
    Conditions::get().error_occurred("Error: no entry point '%' was defined.",
        elms(main_name));
    return FatalError::make(FatalError::feUnexpected);
  } else if (!is<Lambda>(entry_point)) {
    Conditions::get().error_occurred("Entry point '%' is not a function.",
        elms(main_name));
    return FatalError::make(FatalError::feUnexpected);
  }
  ref<Lambda> lambda = protect(cast<Lambda>(entry_point));
  @check ref<Task> task = factory().new_task(architecture());
  architecture().run(lambda, task);
  return Success::make();
}

void Runtime::report_load_error(ImageLoadStatus &info) {
  switch (info.status) {
    case ImageLoadStatus::lsTypeMismatch: {
      static string kErrorMessage =
        "Type mismatch while loading image (%s)\n"
        "  Expected: %s\n"
        "  Found: %s";
      string expected_str = Layout::layout_name(info.error_info.type_mismatch.expected);
      string found_str = Layout::layout_name(info.error_info.type_mismatch.found);
      const char *location = info.error_info.type_mismatch.location;
      Conditions::get().error_occurred(kErrorMessage, elms(location,
          expected_str, found_str));
      return;
    }
    case ImageLoadStatus::lsInvalidImage: {
      static string kErrorMessage = "Invalid image";
      Conditions::get().error_occurred(kErrorMessage, elms());
      return;
    }
    case ImageLoadStatus::lsRootCount: {
      static string kErrorMessage =
        "Invalid root count\n"
        "  Expected: %\n"
        "  Found: %";
      Conditions::get().error_occurred(kErrorMessage,
          elms(info.error_info.root_count.expected,
              info.error_info.root_count.found));
      return;
    }
    case ImageLoadStatus::lsInvalidMagic: {
      static string kErrorMessage =
        "Invalid image\n"
        "  Expected magic number: 0x%{08X}\n"
        "  Found: 0x%{08X}";
      Conditions::get().error_occurred(kErrorMessage,
          elms(Image::kMagicNumber, info.error_info.magic.found));
      return;
    }
    case ImageLoadStatus::lsInvalidVersion: {
      static string kErrorMessage =
        "Invalid image\n"
        "  Expected version: %\n"
        "  Found: %";
      Conditions::get().error_occurred(kErrorMessage,
          elms(Image::kCurrentVersion, info.error_info.version.found));
      return;
    }
    default:
      UNHANDLED(ImageLoadStatus::Status, info.status);
      break;
  }
}

Signal *Runtime::load_image(Image &image) {
  ImageContext info(*this);
  image.initialize(info);
  if (info.has_error()) {
    report_load_error(info.status());
    return FatalError::make(FatalError::feUnexpected);
  }
  Data *roots_val = image.load(info);
  if (info.has_error()) {
    report_load_error(info.status());
    return FatalError::make(FatalError::feUnexpected);
  } else {
    ref_block<> protect(refs());
    Tuple *roots = cast<Tuple>(roots_val);
    return install_loaded_roots(protect(roots));
  }
}

Signal *Runtime::install_loaded_roots(ref<Tuple> roots) {
  for (uword i = 0; i < roots->length(); i++) {
    ref_block<> protect(refs());
    ref<Value> raw_changes = protect(roots->get(i));
    if (is<Smi>(raw_changes)) continue;
    ref<Object> changes = cast<Object>(raw_changes);
    ref<Object> root = get_root(i);
    @try install_object(root, changes);
  }
  return Success::make();
}

Signal *Runtime::install_object(ref<Object> root, ref<Object> changes) {
  InstanceType type = root.type();
  switch (type) {
    case tHashMap:
      return install_hash_map(cast<HashMap>(root), cast<HashMap>(changes));
    case tLayout:
      return install_layout(cast<Layout>(root), cast<Protocol>(changes));
    default:
      UNHANDLED(InstanceType, type);
      return FatalError::make(FatalError::feUnexpected);
  }
}

Signal *Runtime::install_hash_map(ref<HashMap> root, ref<HashMap> changes) {
  ref_block<> protect(refs());
  // First copy all elements into the tables so that we can iterate
  // through the elements independent of whether a gc occurs or not
  uword length = changes.size();
  @check ref<Tuple> keys = factory().new_tuple(length);
  @check ref<Tuple> values = factory().new_tuple(length);
  HashMap::Iterator iter(*changes);
  HashMap::Iterator::Entry entry;
  for (uword i = 0; i < length; i++) {
    bool next_result = iter.next(&entry);
    use(next_result); ASSERT(next_result);
    keys->set(i, entry.key);
    values->set(i, entry.value);
  }
  // Check that we've reached the end
  ASSERT(!iter.next(&entry));
  // Then add the elements to the root object
  for (uword i = 0; i < length; i++) {
    ref_block<> protect(refs());
    root.set(heap(), protect(keys->get(i)), protect(values->get(i)));
  }
  return Success::make();
}

Signal *Runtime::install_layout(ref<Layout> root, ref<Protocol> changes) {
  if (!root->is_empty()) {
    scoped_string str(changes.name()->to_string());
    Conditions::get().error_occurred("Root class % is not empty.", elms(*str));
    return FatalError::make(FatalError::feUnexpected);
  }
  root->set_protocol(*changes);
  ASSERT(!root->is_empty());
  return Success::make();
}

}
