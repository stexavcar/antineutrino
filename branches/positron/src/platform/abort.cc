#include "platform/abort.h"
#include "utils/log.h"
#include "utils/string-inl.h"

namespace neutrino {

abstract_resource *Abort::first_ = NULL;
abstract_resource *Abort::last_ = NULL;

void Abort::register_resource(abstract_resource &that) {
  assert that.prev_ == ((void*) NULL);
  assert that.next_ == ((void*) NULL);
  that.prev_ = last_;
  that.next_ = NULL;
  if (first_ == NULL) first_ = &that;
  if (last_ != NULL) last_->next_ = &that;
  last_ = &that;
}

void Abort::unregister_resource(abstract_resource &that) {
  if (that.prev_ == NULL) first_ = that.next_;
  else that.prev_->next_ = that.next_;
  if (that.next_ == NULL) last_ = that.prev_;
  else that.next_->prev_ = that.prev_;
}

bool Abort::has_cleaned_up_ = false;
void Abort::release_resources() {
  if (has_cleaned_up_) return;
  LOG().info("Releasing resources on abort", args(0));
  has_cleaned_up_ = true;
  while (first_ != NULL)
    first_->dispose();
}

word Abort::resource_count() {
  int count = 0;
  abstract_resource *current = first_;
  while (current != NULL) {
    count++;
    current = current->next_;
  }
  return count;
}

} // namespace neutrino
