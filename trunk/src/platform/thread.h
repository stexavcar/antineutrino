#ifndef _PLATFORM_THREAD
#define _PLATFORM_THREAD


namespace neutrino {


class NativeThread {
public:
  NativeThread();
  virtual ~NativeThread();
  void start();
  void join();
  virtual void run() = 0;

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

private:
  class Data;
  Data *data() { return data_; }
  Data *data_;
};


} // namespace neutrino

#endif // _PLATFORM_THREAD
