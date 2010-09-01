#include "whiteout/wtk.h"

namespace wtk {

Quant Quant::scale(const Quant &from, const Quant &to, float t) {
  @assert from.type_ == to.type_;
  switch (from.type_) {
    case qAbsolute:
      return Quant(static_cast<int>((from.quantity_ * t) + (to.quantity_ * (1 - t))));
    case qScale:
      return Quant(qScale, static_cast<int>(from.quantity_ * t + to.quantity_ * (1 - t)));
    default:
      return Quant();
  }
}

void Animator::tick() {
  for (unsigned i = 0; i < entries().length(); i++) {
    Entry &entry = entries()[i];
    entry.current_ += entry.delta_;
    if (entry.current_ >= entry.limit_) {
      entry.current_ = entry.limit_;
      entry.delta_ = -entry.delta_;
    } else if (entry.current_ < 0) {
      entry.current_ = 0;
      entry.delta_ = -entry.delta_;
    }
    *entry.subject_ = Quant::scale(entry.low_,
        entry.high_, static_cast<float>(entry.current_) / entry.limit_);
  }
}

} // namespace wtk
