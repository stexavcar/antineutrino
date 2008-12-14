#include "cctest/nunit-inl.h"
#include "utils/list-inl.h"
#include "utils/worklist-inl.pp.h"

using namespace neutrino;

TEST(simple) {
  DoublyLinkedList<int> list;
  @check list.is_empty();
  @check list.size() == 0;

  list.add_first(4);
  @check !list.is_empty();
  @check list.size() == 1;
  @check list.last() == 4;
  @check list.first() == 4;
  @check list.remove_first() == 4;
  @check list.is_empty();

  list.add_last(6);
  @check list.size() == 1;
  @check list.last() == 6;
  @check list.first() == 6;
  @check list.remove_first() == 6;
  @check list.is_empty();

  for (int i = 0; i < 10; i++) {
    list.add_first(i);
    @check list.size() == (i + 1);
    @check list.first() == i;
    @check list.last() == 0;
  }
  for (int i = 0; i < 10; i++) {
    @check list.size() == 10 - i;
    @check list.first() == 9;
    @check list.remove_last() == i;
  }
  @check list.is_empty();
  for (int i = 0; i < 10; i++) {
    list.add_last(i);
    @check list.size() == (i + 1);
    @check list.first() == 0;
    @check list.last() == i;
  }
  for (int i = 0; i < 10; i++) {
    @check list.size() == 10 - i;
    @check list.last() == 9;
    @check list.remove_first() == i;
  }
}

class SharedData {
public:
  SharedData() {
    mutex().initialize();
    worklist().initialize();
    for (uword i = 0; i < actions_performed().length(); i++)
      actions_performed()[i] = false;
  }
  void set(int i) {
    Mutex::With with(mutex());
    @assert !actions_performed()[i];
    actions_performed()[i] = true;
  }
  bool get(int i) {
    Mutex::With with(mutex());
    return actions_performed()[i];
  }
  WorkList<int> &worklist() { return worklist_; }
  static const int kActionCount = 1 << 14;
private:
  list_value<bool, kActionCount> actions_performed_;
  list<bool> &actions_performed() { return actions_performed_; }
  Mutex &mutex() { return mutex_; }
  Mutex mutex_;
  WorkList<int> worklist_;
};

class MultiTestThread : public NativeThread {
public:
  MultiTestThread() : data_(NULL) { }
  void initialize(SharedData &data, int index) {
    data_ = &data;
    index_ = index;
  }
  virtual void run();
  SharedData &data() { return *data_; }
  int index() { return index_; }
private:
  SharedData *data_;
  int index_;
};

void MultiTestThread::run() {
  while (true) {
    int value = data().worklist().take();
    if (value == -1) break;
    data().set(value);
  }
}

TEST(multi_thread) {
  SharedData data;
  const int kThreadCount = 10;
  MultiTestThread threads[kThreadCount];
  for (int i = 0; i < kThreadCount; i++) {
    threads[i].initialize(data, i);
    threads[i].start();
  }
  for (int i = 0; i < SharedData::kActionCount; i++)
    data.worklist().offer(i);
  for (int i = 0; i < kThreadCount; i++)
    data.worklist().offer(-1);
  for (int i = 0; i < kThreadCount; i++)
    threads[i].join();
  for (int i = 0; i < SharedData::kActionCount; i++)
    @check data.get(i);
}
