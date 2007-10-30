#include "heap/zone-inl.h"

namespace neutrino {
namespace zone {

Zone *Zone::current_ = NULL;

Zone::Zone() : prev_(current_) {
  current_ = this;
  arena_ = new Arena(NULL);
}

Zone::~Zone() {
  current_ = prev_;
  Arena *current = arena_;
  while (current != NULL) {
    Arena *arena = current;
    current = current->prev();
    delete arena;
  }
}

int8_t *Arena::allocate(size_t size) {
  ASSERT(size < kSize);
  if (cursor_ + size < kSize) {
    int8_t *result = memory_ + cursor_;
    cursor_ += size;
    return result;
  } else {
    return NULL;
  }
}

int8_t *Zone::allocate(size_t size) {
  int8_t *result = arena_->allocate(size);
  if (result == NULL) {
    arena_ = new Arena(arena_);
    result = arena_->allocate(size);
    ASSERT(result != NULL);
  }
  return result;
}

size_t Zone::bytes_allocated() {
  size_t result = 0;
  Arena *current = arena_;
  while (current != NULL) {
    result += current->bytes_allocated();
    current = current->prev();
  }
  return result;
}

} // zone
} // neutrino
