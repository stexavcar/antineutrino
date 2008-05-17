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
  virtual void run(ref<Lambda> lambda, ref<Task> task) = 0;
  virtual void disassemble(Lambda *lambda, string_buffer &buf) = 0;
};

class BytecodeArchitecture : public Architecture {
public:
  BytecodeArchitecture(Runtime &runtime) : interpreter_(runtime) { }
  virtual void run(ref<Lambda> lambda, ref<Task> task);
  virtual void disassemble(Lambda *lambda, string_buffer &buf);
private:
  Interpreter &interpreter() { return interpreter_; }
  Interpreter interpreter_;
};

/**
 * A neutrino runtime encapsulated in a single object.
 */
class Runtime : public nocopy {
public:
  Runtime(DynamicLibraryCollection *dylibs = 0);
  bool initialize(Architecture *arch);

  bool load_image(Image &image);
  void report_load_error(ImageLoadStatus &info);
  bool install_loaded_roots(ref<Tuple> roots);
  bool install_object(ref<Object> root, ref<Object> changes);
  bool install_hash_map(ref<HashMap> root, ref<HashMap> changes);
  bool install_layout(ref<Layout> root, ref<Protocol> changes);
  void start();

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
