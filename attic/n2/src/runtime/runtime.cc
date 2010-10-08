#include "heap/ref-inl.h"
#include "io/image.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"
#include "utils/log.h"
#include "utils/string-inl.pp.h"
#include "utils/worklist-inl.pp.h"
#include "values/values-inl.pp.h"

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

likely Runtime::initialize(Architecture *arch) {
  worklist().initialize();
  sync_action_mutex().initialize();
  possibly init_result = roots().initialize(heap());
  if (init_result.has_failed()) {
    init_result.signal()->to_string().println();
    if (is<FatalError>(init_result.signal())) {
      return cast<FatalError>(init_result.signal());
    } else {
      return FatalError::make(FatalError::feInitialization);
    }
  }
  architecture_ = arch;
  if (arch != NULL) {
    possibly setup_result = architecture().setup(*this);
    if (setup_result.has_failed()) {
      if (is<FatalError>(setup_result.signal())) {
        return cast<FatalError>(setup_result.signal());
      } else {
        return FatalError::make(FatalError::feInitialization);
      }
    }
  }
  return Success::make();
}

likely RunMain::run(Runtime &runtime) {
  ref_block<> protect(runtime.refs());
  @check(probably) ref<String> main_name = runtime.factory().new_string("entry_point");
  Data *entry_point = runtime.roots().toplevel()->get(*main_name);
  if (is<Nothing>(entry_point)) {
    LOG().error("No entry point '%' was defined.", elms(main_name));
    return FatalError::make(FatalError::feUnexpected);
  } else if (!is<Lambda>(entry_point)) {
    LOG().error("Entry point '%' is not a function.",
        elms(main_name));
    return FatalError::make(FatalError::feUnexpected);
  }
  ref<Lambda> lambda = protect(cast<Lambda>(entry_point));
  @check(probably) ref<Task> task = runtime.factory().new_task(runtime.architecture());
  runtime.architecture().run(lambda, task);
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
      LOG().error(kErrorMessage, elms(location,
          expected_str, found_str));
      return;
    }
    case ImageLoadStatus::lsInvalidImage: {
      static string kErrorMessage = "Invalid image";
      LOG().error(kErrorMessage, elms());
      return;
    }
    case ImageLoadStatus::lsRootCount: {
      static string kErrorMessage =
        "Invalid root count\n"
        "  Expected: %\n"
        "  Found: %";
      LOG().error(kErrorMessage,
          elms(info.error_info.root_count.expected,
              info.error_info.root_count.found));
      return;
    }
    case ImageLoadStatus::lsInvalidMagic: {
      static string kErrorMessage =
        "Invalid image\n"
        "  Expected magic number: 0x%{08X}\n"
        "  Found: 0x%{08X}";
      LOG().error(kErrorMessage,
          elms(Image::kMagicNumber, info.error_info.magic.found));
      return;
    }
    case ImageLoadStatus::lsInvalidVersion: {
      static string kErrorMessage =
        "Invalid image\n"
        "  Expected version: %\n"
        "  Found: %";
      LOG().error(kErrorMessage,
          elms(Image::kCurrentVersion, info.error_info.version.found));
      return;
    }
    default:
      UNHANDLED(ImageLoadStatus::Status, info.status);
      break;
  }
}

likely Runtime::load_image(Image &image) {
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

likely Runtime::install_loaded_roots(ref<Tuple> roots) {
  for (uword i = 0; i < roots->length(); i++) {
    ref_block<> protect(refs());
    ref<Value> raw_changes = protect(roots->get(i));
    if (is<Smi>(raw_changes)) continue;
    ref<Object> changes = cast<Object>(raw_changes);
    ref<Object> root = get_root(i);
    @try(likely) install_object(root, changes);
  }
  return Success::make();
}

likely Runtime::install_object(ref<Object> root, ref<Object> changes) {
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

likely Runtime::install_hash_map(ref<HashMap> root, ref<HashMap> changes) {
  ref_block<> protect(refs());
  // First copy all elements into the tables so that we can iterate
  // through the elements independent of whether a gc occurs or not
  uword length = changes.size();
  @check(probably) ref<Tuple> keys = factory().new_tuple(length);
  @check(probably) ref<Tuple> values = factory().new_tuple(length);
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

likely Runtime::install_layout(ref<Layout> root, ref<Protocol> changes) {
  if (!root->is_empty()) {
    scoped_string str(changes.name()->to_string());
    LOG().error("Root class % is not empty.", elms(*str));
    return FatalError::make(FatalError::feUnexpected);
  }
  root->set_protocol(*changes);
  ASSERT(!root->is_empty());
  return Success::make();
}

likely Runtime::start(bool stop_when_empty) {
  while (true) {
    if (stop_when_empty && worklist().is_empty())
      return Success::make();
    Action *action = worklist().take();
    if (action == NULL) {
      return Success::make();
    } else if (action->is_synchronous()) {
      Mutex::With with(sync_action_mutex());
      action->begin().signal();
      action->end().wait(sync_action_mutex());
    } else {
      @try(likely) action->run(*this);
    }
  }
}

void Runtime::schedule_interrupt() {
  worklist().offer(NULL);
}

void Runtime::schedule_async(Action &action) {
  action.is_synchronous_ = false;
  worklist().offer(&action);
}

likely Runtime::schedule_sync(Action &action) {
  action.is_synchronous_ = true;
  action.begin().initialize();
  action.end().initialize();
  // Grab the synchronous action mutex before doing anything.
  Mutex::With with(sync_action_mutex());
  worklist().offer(&action);
  // When the scheduler gets to this action the is_synchronous flag
  // tells it not to execute it but to signal the condition.  Wait for
  // that to happen.
  action.begin().wait(sync_action_mutex());
  // After signalling the scheduler will wait for the end signal before
  // continuing.  That's when we can execute the action.
  likely result = action.run(*this);
  // Tell the scheduler that it can continue.
  action.end().signal();
  return result;
}

} // namespace neutrino