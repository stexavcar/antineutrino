#ifndef MONITOR_MONITOR_INL
#define MONITOR_MONITOR_INL

#include "monitor/monitor.h"
#include "utils/types-inl.h"

namespace neutrino {

template <class Type>
Watch<Type>::Watch(string name)
    : AbstractWatch(name, sizeof(Type)) {
  initialize_monitor(&local_instance_);
}

template <class Type>
void Watch<Type>::initialize_monitor(void *data) {
  monitor_ = static_cast<Type*>(data);
  monitor_->clear();
}

template <class Type>
void Watch<Type>::write_on(string_buffer &buf) {
  monitor_->write_on(name(), buf);
}

void Counter::increment() {
  count_++;
}

void Counter::decrement() {
  count_--;
}

void HighWaterMark::increment() {
  count_++;
  if (count_ > high_water_mark_)
    high_water_mark_ = count_;
}

void HighWaterMark::decrement() {
  count_--;
}

} // neutrino

#endif // MONITOR_MONITOR_INL
