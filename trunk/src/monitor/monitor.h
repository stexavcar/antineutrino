#ifndef _MONITOR_MONITOR
#define _MONITOR_MONITOR

#include "utils/string.h"

namespace neutrino {

typedef void *(WatchMalloc)(size_t size);

class WatchData {
public:
  explicit WatchData(AbstractWatch &holder);
  static void initialize(WatchMalloc *watch_malloc);
private:
  WatchData *next() { return next_; }
  AbstractWatch &holder() { return holder_; }
  WatchData *next_;
  AbstractWatch &holder_;

  static WatchData *first_;
  static const uword kNameLength = 32;
  
  friend class Monitor;
};

enum MonitorType {
  COUNTER, HIGH_WATER_MARK
};

class Monitor {
public:
  void initialize(MonitorType type, uword size);
  static void write_on(string_buffer &buf);
private:
  MonitorType type_;
  uword size_;
};

struct Counter : public Monitor {
public:
  void clear();
  inline void increment();
  inline void decrement();
  void write_on(string name, string_buffer &buf);
private:
  uword count_;
};

struct HighWaterMark : public Monitor {
  void clear();
  inline void increment();
  inline void decrement();
  void write_on(string name, string_buffer &buf);
private:
  uword count_;
  uword high_water_mark_;
};

class AbstractWatch {
public:
  explicit AbstractWatch(string name, uword monitor_size);
  virtual void initialize_monitor(void *data) = 0;
  uword monitor_size() { return monitor_size_; }
  string name() { return name_; }
  virtual void write_on(string_buffer &buf) = 0;
private:
  string name_;
  WatchData data_;
  uword monitor_size_;
};

template <class Type>
class Watch : public AbstractWatch {
public:
  explicit Watch(string name);
  inline Type *operator->() { return monitor_; }
  virtual void initialize_monitor(void *data);
  virtual void write_on(string_buffer &buf);
private:
  Type *monitor_;
  Type local_instance_;
};

} // neutrino
 
#endif // _MONITOR_MONITOR
