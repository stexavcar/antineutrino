#include "runtime/ref-inl.h"
#include "utils/buffer-inl.h"

namespace neutrino {

RefManager::RefManager()
    : top_(NULL) { }

void RefManager::dispose(persistent_cell &cell) {
  word last_index = persistent_list().length() - 1;
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

} // namespace neutrino
