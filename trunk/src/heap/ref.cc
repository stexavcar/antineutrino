#include "heap/ref-inl.h"
#include "monitor/monitor-inl.h"
#include "utils/list-inl.h"

namespace neutrino {


RefStack::RefStack() 
    : spare_block_(NULL) { }


Value **RefStack::grow() {
  ASSERT_C(cnNoRefScope, current().block_count >= 0);
  ref_block *extension;
  if (spare_block() == NULL) {
    extension = new ref_block();
  } else {
    extension = spare_block();
    spare_block_ = NULL;
  }
  block_stack().push(extension);
  current().block_count++;
  Value **result = extension->first_cell();
  current().next_cell = result + 1;
  current().limit = extension->limit();
  return result;
}


void RefStack::shrink() {
  ASSERT(current().block_count != 0);
  uword blocks_to_delete = current().block_count;
  if (spare_block() == NULL) {
    spare_block_ = block_stack().pop();
    blocks_to_delete--;
  }
  for (uword i = 0; i < blocks_to_delete; i++) {
    delete block_stack().pop();
  }
}


}
