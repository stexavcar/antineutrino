#ifndef _RUNTIME
#define _RUNTIME

#include "heap/factory.h"
#include "heap/heap.h"
#include "heap/ref.h"
#include "heap/roots.h"
#include "platform/dylib.h"
#include "runtime/interpreter.h"

namespace neutrino {

/**
 * A neutrino runtime encapsulated in a single object.
 */
class Runtime {
public:
  Runtime(DynamicLibraryCollection *dylibs = 0);
  bool initialize();
  Factory &factory() { return factory_; }
  Heap &heap() { return heap_; }
  Roots &roots() { return roots_; }
  bool load_image(Image &image);
  void report_load_error(ImageLoadStatus &info);
  bool install_loaded_roots(ref<Tuple> roots);
  bool install_object(ref<Object> root, ref<Object> changes);
  bool install_dictionary(ref<Dictionary> root, ref<Dictionary> changes);
  bool install_layout(ref<Layout> root, ref<Protocol> changes);
  Interpreter &interpreter() { return interpreter_; }
  void start();

  // Declare root field ref accessors
  #define DECLARE_ROOT_ACCESSOR(n, Type, name, Name, allocator)      \
    ref<Type> name() { return ref<Type>(&roots().name()); }
  eRoots(DECLARE_ROOT_ACCESSOR)
  #undef DECLARE_ROOT_ACCESSOR
  
  inline ref<Object> get_root(uword index);
  
  DynamicLibraryCollection *dylibs() { return dylibs_; }
  RefStack &refs() { return refs_; }
  
private:
  Roots roots_;
  Heap heap_;
  Factory factory_;
  Interpreter interpreter_;
  DynamicLibraryCollection *dylibs_;
  RefStack refs_;
};

} // neutrino

#endif // _RUNTIME
