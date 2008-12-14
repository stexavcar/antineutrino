#ifndef _UTILS_WORKLIST
#define _UTILS_WORKLIST

#include "utils/types.h"
#include "platform/thread.h"

namespace neutrino {

template <typename T>
class ListNode {
public:
  ListNode(const T &data, ListNode<T> *prev, ListNode<T> *next)
    : data_(data)
    , prev_(prev)
    , next_(next) { }
  ListNode<T> *prev() { return prev_; }
  void set_prev(ListNode<T> *v) { prev_ = v; }
  ListNode<T> *next() { return next_; }
  void set_next(ListNode<T> *v) { next_ = v; }
  const T &data() { return data_; }
private:
  T data_;
  ListNode<T> *prev_;
  ListNode<T> *next_;
};

template <typename T>
class DoublyLinkedList {
public:
  DoublyLinkedList() : head_(NULL), tail_(NULL), size_(0) { }
  void add_first(const T &e);
  void add_last(const T &e);
  T remove_first();
  T remove_last();
  const T &first() { return head()->data(); }
  const T &last() { return tail()->data(); }
  int size() { return size_; }
  bool is_empty() { return size_ == 0; }
private:
  ListNode<T> *head() { return head_; }
  void set_head(ListNode<T> *v) { head_ = v; }
  ListNode<T> *tail() { return tail_; }
  void set_tail(ListNode<T> *v) { tail_ = v; }
  ListNode<T> *head_;
  ListNode<T> *tail_;
  int size_;

};

class Action {

};

template <typename T>
class WorkList {
public:
  void initialize() {
    mutex().initialize();
    added().initialize();
  }
  void offer(const T &e) {
    Mutex::With with(mutex());
    elements().add_last(e);
    added().signal();
  }
  T take() {
    Mutex::With with(mutex());
    while (elements().is_empty())
      added().wait(mutex());
    return elements().remove_first();
  }
private:
  DoublyLinkedList<T> &elements() { return elements_; }
  Mutex &mutex() { return mutex_; }
  ConditionVariable &added() { return added_; }
  DoublyLinkedList<T> elements_;
  Mutex mutex_;
  ConditionVariable added_;
};

} // namespace neutrino

#endif // _UTILS_WORKLIST
