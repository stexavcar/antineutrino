#include <cstdlib>
#include "whiteout/backend.h"

namespace whiteout {

RegisterBackend *Backend::first_ = NULL;

Backend *Backend::create(CairoPainter &painter) {
  RegisterBackend *constructor = first_;
  if (!constructor) return NULL;
  return constructor->create(painter);
}

RegisterBackend::RegisterBackend(backend_constructor_t *cons)
    : cons_(cons) {
  next_ = Backend::first_;
  Backend::first_ = this;
}

} // whiteout
