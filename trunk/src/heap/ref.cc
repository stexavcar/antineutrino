#include "heap/ref-inl.h"
#include "monitor/monitor-inl.h"
#include "utils/list-inl.h"

namespace neutrino {


RefManager::RefManager()
    : top_(NULL) { }


void RefManager::dispose(persistent_cell &cell) {
  uword last_index = persistent_list().length() - 1;
  if (cell.index() < last_index) {
    // If this is not the last persistent cell then we override its
    // entry in the list with the last one, to avoid fragmentation
    persistent_cell *last_cell = persistent_list()[last_index];
    last_cell->set_index(cell.index());
    persistent_list()[cell.index()] = last_cell;
  }
  persistent_list().pop();
  delete &cell;
}


}
