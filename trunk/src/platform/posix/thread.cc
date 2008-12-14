#include "platform/thread.h"

#include <pthread.h>

namespace neutrino {

class NativeThread::Data {
public:
  pthread_t &thread() { return thread_; }
private:
  pthread_t thread_;
};

NativeThread::NativeThread()
  : data_(new Data()) { }

NativeThread::~NativeThread() {
  delete data();
}

static void *run_bridge(void *arg) {
  static_cast<NativeThread*>(arg)->run();
  return NULL;
}

void NativeThread::start() {
  pthread_create(&data()->thread(), NULL, run_bridge, this);
}

void NativeThread::join() {
  void *value = NULL;
  pthread_join(data()->thread(), &value);
}

class Mutex::Data {
public:
  pthread_mutex_t &mutex() { return mutex_; }
private:
  pthread_mutex_t mutex_;
};

Mutex::Mutex()
  : data_(new Data()) { }

Mutex::~Mutex() {
  pthread_mutex_destroy(&data()->mutex());
  delete data();
}

void Mutex::initialize() {
  pthread_mutex_init(&data()->mutex(), NULL);
}

void Mutex::lock() {
  pthread_mutex_lock(&data()->mutex());
}

void Mutex::unlock() {
  pthread_mutex_unlock(&data()->mutex());
}

class ConditionVariable::Data {
public:
  pthread_cond_t &cond() { return cond_; }
private:
  pthread_cond_t cond_;
};

ConditionVariable::ConditionVariable()
  : data_(new Data()) { }

ConditionVariable::~ConditionVariable() {
  pthread_cond_destroy(&data()->cond());
  delete data_;
}

void ConditionVariable::wait(Mutex &mutex) {
  pthread_cond_wait(&data()->cond(), &mutex.data()->mutex());
}

void ConditionVariable::signal() {
  pthread_cond_signal(&data()->cond());
}

void ConditionVariable::initialize() {
  pthread_cond_init(&data()->cond(), NULL);
}

} // neutrino
