#include "monitor/monitor-inl.h"
#include "utils/checks.h"

namespace neutrino {

WatchData *WatchData::first_ = NULL;

WatchData::WatchData(AbstractWatch &holder)
    : next_(first_)
    , holder_(holder) { 
  first_ = this;
}

AbstractWatch::AbstractWatch(string name, uint32_t monitor_size)
    : name_(name)
    , data_(*this)
    , monitor_size_(monitor_size) { }

void WatchData::initialize(WatchMalloc *monitor_malloc) {
  WatchData *current = first_;
  while (current != NULL) {
    AbstractWatch &watch = current->holder();
    void *memory = monitor_malloc(watch.monitor_size());
    watch.initialize_monitor(memory);
    current = current->next();
  }
}

void Monitor::initialize(MonitorType type, uint32_t size) {
  type_ = type;
  size_ = size;
}

void Monitor::write_on(string_buffer &buf) {
  WatchData *current = WatchData::first_;
  while (current != NULL) {
    AbstractWatch &watch = current->holder();
    watch.write_on(buf);
    current = current->next();
  }
}

void Counter::clear() {
  Monitor::initialize(COUNTER, sizeof(Counter));
  count_ = 0;
}

void Counter::write_on(string name, string_buffer &buf) {
  buf.printf("%: %\n", name.chars(), count_);
}

void HighWaterMark::clear() {
  Monitor::initialize(HIGH_WATER_MARK, sizeof(HighWaterMark));
  count_ = 0;
  high_water_mark_ = 0;
}

void HighWaterMark::write_on(string name, string_buffer &buf) {
  buf.printf("%: %, max: %\n", name.chars(), count_, high_water_mark_);
}

}
