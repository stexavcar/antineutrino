#include "platform/abort.h"
#include "utils/log.h"
#include "utils/string-inl.h"

namespace neutrino {

Finalized *Abort::first_ = NULL;
Finalized *Abort::last_ = NULL;

void Finalized::install() {
  prev_ = Abort::last_;
  next_ = NULL;
  if (Abort::first_ == NULL) Abort::first_ = this;
  if (Abort::last_ != NULL) Abort::last_->next_ = this;
  Abort::last_ = this;
}

void Finalized::uninstall() {
  if (prev_ == NULL) Abort::first_ = next_;
  else prev_->next_ = next_;
  if (next_ == NULL) Abort::last_ = prev_;
  else next_->prev_ = prev_;
}

Finalized::Finalized() : has_been_finalized_(false) {
  install();
}

Finalized::~Finalized() {
  uninstall();
  has_been_finalized_ = true;
}

bool Abort::has_cleaned_up_ = false;
void Abort::finalize() {
  if (has_cleaned_up_) return;
  LOG().info("Finalizing objects", args(0));
  has_cleaned_up_ = true;
  Finalized *current = Abort::first_;
  while (current != NULL) {
    current->cleanup();
    current = current->next_;
  }
}

} // namespace neutrino

