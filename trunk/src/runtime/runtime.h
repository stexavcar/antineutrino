#ifndef _RUNTIME
#define _RUNTIME

#include "heap/factory.h"
#include "heap/heap.h"
#include "heap/ref.h"
#include "heap/roots.h"
#include "runtime/interpreter.h"

namespace neutrino {

/**
 * A neutrino runtime encapsulated in a single object.
 */
class Runtime {
public:
  Runtime();
  bool initialize();
  Factory &factory() { return factory_; }
  Heap &heap() { return heap_; }
  Roots &roots() { return roots_; }
  bool load_image(Image &image);
  bool install_loaded_roots(ref<Tuple> roots);
  bool install_object(ref<Object> root, ref<Object> changes);
  bool install_dictionary(ref<Dictionary> root, ref<Dictionary> changes);
  bool install_class(ref<Class> root, ref<Class> changes);
  Interpreter &interpreter() { return interpreter_; }
  void start();

  // Declare root field ref accessors
  #define DECLARE_ROOT_ACCESSOR(n, Type, name, Name, allocator)      \
    ref<Type> name() { return ref<Type>(&roots().name()); }
  FOR_EACH_ROOT(DECLARE_ROOT_ACCESSOR)
  #undef DECLARE_ROOT_ACCESSOR
  
  inline ref<Object> get_root(uint32_t index);
  
  /**
   * Stack-allocated class that sets the given runtime as the current
   * one and reestablishes the previous one when exiting the scope.
   */
  class Scope {
  public:
    inline Scope(Runtime &runtime);
    inline ~Scope();
  private:
    Runtime *previous_;
  };
  
  static inline Runtime &current();
  
private:
  static Runtime *current_;
  Roots roots_;
  Heap heap_;
  Factory factory_;
  Interpreter interpreter_;
};

}

#endif // _RUNTIME
