#ifndef _RUNTIME
#define _RUNTIME

#include "compiler/ast.h"
#include "heap/gc-safe.h"
#include "heap/heap.h"
#include "heap/ref.h"
#include "heap/roots.h"
#include "platform/dylib.h"
#include "runtime/interpreter.h"
#include "utils/worklist.h"

namespace neutrino {

class Architecture {
public:
  Architecture(Runtime &runtime) : runtime_(runtime) { }
  virtual possibly setup(Runtime &runtime) = 0;
  virtual void dispose() = 0;
  virtual void run(ref<Lambda> lambda, ref<Task> task) = 0;
  virtual void disassemble(Lambda *lambda, string_buffer &buf) = 0;
  virtual possibly initialize_task(Task *task) = 0;
protected:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

class BytecodeArchitecture : public Architecture {
public:
  BytecodeArchitecture(Runtime &runtime)
      : Architecture(runtime)
      , interpreter_(runtime) { }
  virtual possibly setup(Runtime &runtime);
  virtual void dispose();
  virtual void run(ref<Lambda> lambda, ref<Task> task);
  virtual void disassemble(Lambda *lambda, string_buffer &buf);
  virtual possibly initialize_task(Task *task);
private:
  Interpreter &interpreter() { return interpreter_; }
  persistent<Lambda> bottom() { return bottom_; }
  Interpreter interpreter_;
  persistent<Lambda> bottom_;
};

class Action {
public:
  Action() : is_synchronous_(false) { }
  virtual likely run(Runtime &runtime) = 0;
private:
  friend class Runtime;
  ConditionVariable &begin() { return begin_; }
  ConditionVariable &end() { return end_; }
  bool is_synchronous() { return is_synchronous_; }
  bool is_synchronous_;
  ConditionVariable begin_;
  ConditionVariable end_;
};

class RunMain : public Action {
public:
  virtual likely run(Runtime &runtime);
};

/**
 * A neutrino runtime encapsulated in a single object.
 */
class Runtime : public nocopy {
public:
  Runtime(DynamicLibraryCollection *dylibs = 0);
  ~Runtime();
  likely initialize(Architecture *arch);

  likely load_image(Image &image);
  likely install_loaded_roots(ref<Tuple> roots);
  likely install_object(ref<Object> root, ref<Object> changes);
  likely install_hash_map(ref<HashMap> root, ref<HashMap> changes);
  likely install_layout(ref<Layout> root, ref<Protocol> changes);
  likely start(bool stop_when_empty);

  void report_load_error(ImageLoadStatus &info);

  // Declare root field ref accessors
  #define DECLARE_ROOT_ACCESSOR(n, Type, name, Name, allocator)      \
    ref<Type> name() { return ref<Type>(&roots().name()); }
  eRoots(DECLARE_ROOT_ACCESSOR)
  #undef DECLARE_ROOT_ACCESSOR

  inline ref<Object> get_root(uword index);

  // Schedules an action that causes the scheduler to stop processing
  // actions and return.
  void schedule_interrupt();

  // Schedules an action to be executed eventually and returns
  // immediately.  The action will be executed by the thread that
  // started the runtime.
  void schedule_async(Action &action);

  // Schedules an action to be executed eventually and blocks util
  // the action has been executed.  The action will be executed by
  // the calling thread.
  likely schedule_sync(Action &action);

  DynamicLibraryCollection *dylibs() { return dylibs_; }
  RefManager &refs() { return refs_; }
  Architecture &architecture() { return *architecture_; }
  Factory &factory() { return factory_; }
  GcSafe &gc_safe() { return gc_safe_; }
  Heap &heap() { return heap_; }
  Roots &roots() { return roots_; }

private:
  WorkList<Action*> &worklist() { return worklist_; }
  Mutex &sync_action_mutex() { return sync_action_mutex_; }

  Roots roots_;
  Heap heap_;
  Factory factory_;
  GcSafe gc_safe_;
  Architecture *architecture_;
  DynamicLibraryCollection *dylibs_;
  RefManager refs_;
  WorkList<Action*> worklist_;
  Mutex sync_action_mutex_;
};

} // neutrino

#endif // _RUNTIME
