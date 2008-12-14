#ifndef _UTILS_WORKLIST_INL
#define _UTILS_WORKLIST_INL

#include "utils/worklist.h"

namespace neutrino {

template <typename T>
void DoublyLinkedList<T>::add_first(const T &e) {
  ListNode<T> *first = head();
  ListNode<T> *node = new ListNode<T>(e, NULL, first);
  set_head(node);
  if (tail() == NULL)
    set_tail(node);
  else
    first->set_prev(node);
  size_++;
}

template <typename T>
void DoublyLinkedList<T>::add_last(const T &e) {
  ListNode<T> *last = tail();
  ListNode<T> *node = new ListNode<T>(e, last, NULL);
  set_tail(node);
  if (head() == NULL)
    set_head(node);
  else
    last->set_next(node);
  size_++;
}

template <typename T>
T DoublyLinkedList<T>::remove_first() {
  @assert !is_empty();
  ListNode<T> *first = head();
  ListNode<T> *next = first->next();
  set_head(next);
  if (next == NULL)
    set_tail(NULL);
  else
    next->set_prev(NULL);
  size_--;
  T result = first->data();
  delete first;
  return result;
}

template <typename T>
T DoublyLinkedList<T>::remove_last() {
  @assert !is_empty();
  ListNode<T> *last = tail();
  ListNode<T> *prev = last->prev();
  set_tail(prev);
  if (prev == NULL)
    set_head(NULL);
  else
    prev->set_next(NULL);
  size_--;
  T result = last->data();
  delete last;
  return result;
}

template <typename T>
void WorkList<T>::initialize() {
  mutex().initialize();
  added().initialize();
}

template <typename T>
bool WorkList<T>::is_empty() {
  Mutex::With with(mutex());
  return elements().is_empty();
}

template <typename T>
void WorkList<T>::offer(const T &e) {
  Mutex::With with(mutex());
  elements().add_last(e);
  added().signal();
}

template <typename T>
T WorkList<T>::take() {
  Mutex::With with(mutex());
  while (elements().is_empty())
    added().wait(mutex());
  return elements().remove_first();
}

} // namespace neutrino

#endif // _UTILS_WORKLIST_INL
