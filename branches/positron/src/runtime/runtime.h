#ifndef _RUNTIME_RUNTIME
#define _RUNTIME_RUNTIME

#include "runtime/heap.h"
#include "runtime/ref.h"
#include "runtime/roots.h"

namespace neutrino {

class Runtime : public nocopy {
public:
  Runtime() : heap_(*this) { }
  boole initialize();
  Heap &heap() { return heap_; }
  RefManager &refs() { return refs_; }
  Roots &roots() { return roots_; }
private:
  Heap heap_;
  RefManager refs_;
  Roots roots_;
};

} // namespace neutrino

#endif // _RUNTIME_RUNTIME
