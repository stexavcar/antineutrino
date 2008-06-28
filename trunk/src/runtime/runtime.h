#ifndef _RUNTIME
#define _RUNTIME

#include "compiler/ast.h"
#include "heap/gc-safe.h"
#include "heap/heap.h"
#include "heap/ref.h"
#include "heap/roots.h"
#include "platform/dylib.h"
#include "runtime/interpreter.h"

namespace neutrino {

class Architecture {
public:
  Architecture(Runtime &runtime) : runtime_(runtime) { }
  virtual Signal *setup(Runtime &runtime) = 0;
  virtual void dispose() = 0;
  virtual void run(ref<Lambda> lambda, ref<Task> task) = 0;
  virtual void disassemble(Lambda *lambda, string_buffer &buf) = 0;
  virtual Signal *initialize_task(Task *task) = 0;
protected:
  Runtime &runtime() { return runtime_; }
  Runtime &runtime_;
};

class BytecodeArchitecture : public Architecture {
public:
  BytecodeArchitecture(Runtime &runtime)
      : Architecture(runtime)
      , interpreter_(runtime) { }
  virtual Signal *setup(Runtime &runtime);
  virtual void dispose();
  virtual void run(ref<Lambda> lambda, ref<Task> task);
  virtual void disassemble(Lambda *lambda, string_buffer &buf);
  virtual Signal *initialize_task(Task *task);
private:
  Interpreter &interpreter() { return interpreter_; }
  persistent<Lambda> bottom() { return bottom_; }
  Interpreter interpreter_;
  persistent<Lambda> bottom_;
};

/**
 * A neutrino runtime encapsulated in a single object.
 */
class Runtime : public nocopy {
public:
  Runtime(DynamicLibraryCollection *dylibs = 0);
  ~Runtime();
  Signal *initialize(Architecture *arch);

  Signal *load_image(Image &image);
  void report_load_error(ImageLoadStatus &info);
  Signal *install_loaded_roots(ref<Tuple> roots);
  Signal *install_object(ref<Object> root, ref<Object> changes);
  Signal *install_hash_map(ref<HashMap> root, ref<HashMap> changes);
  Signal *install_layout(ref<Layout> root, ref<Protocol> changes);
  Signal *start();

  // Declare root field ref accessors
  #define DECLARE_ROOT_ACCESSOR(n, Type, name, Name, allocator)      \
    ref<Type> name() { return ref<Type>(&roots().name()); }
  eRoots(DECLARE_ROOT_ACCESSOR)
  #undef DECLARE_ROOT_ACCESSOR

  inline ref<Object> get_root(uword index);

  DynamicLibraryCollection *dylibs() { return dylibs_; }
  RefStack &refs() { return refs_; }
  Architecture &architecture() { return *architecture_; }
  Factory &factory() { return factory_; }
  GcSafe &gc_safe() { return gc_safe_; }
  Heap &heap() { return heap_; }
  Roots &roots() { return roots_; }


private:
  Roots roots_;
  Heap heap_;
  Factory factory_;
  GcSafe gc_safe_;
  Architecture *architecture_;
  DynamicLibraryCollection *dylibs_;
  RefStack refs_;
};

} // neutrino

#endif // _RUNTIME
