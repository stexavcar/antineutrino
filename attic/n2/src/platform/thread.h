#ifndef _PLATFORM_THREAD
#define _PLATFORM_THREAD


#include "utils/globals.h"


namespace neutrino {


class NativeThread {
public:
  NativeThread();
  virtual ~NativeThread();
  void start();
  likely join();
  virtual likely run() = 0;

private:
  class Data;
  Data *data() { return data_; }
  Data *data_;
};


class Mutex {
public:
  Mutex();
  ~Mutex();
  void initialize();
  void lock();
  void unlock();
  bool initialized() { return data() != NULL; }

  class With {
  public:
    explicit With(Mutex &mutex) : mutex_(mutex) { mutex.lock(); }
    ~With() { mutex().unlock(); }
  private:
    Mutex &mutex() { return mutex_; }
    Mutex &mutex_;
  };

private:
  friend class ConditionVariable;
  class Data;
  Data *data() { return data_; }
  Data *data_;
};


class ConditionVariable {
public:
  ConditionVariable();
  ~ConditionVariable();
  void wait(Mutex &mutex);
  void signal();
  void initialize();
  bool initialized() { return data() != NULL; }

private:
  class Data;
  Data *data() { return data_; }
  Data *data_;
};


} // namespace neutrino

#endif // _PLATFORM_THREAD
