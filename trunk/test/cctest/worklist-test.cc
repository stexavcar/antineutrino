#include "cctest/nunit-inl.h"
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
