#include "heap/ref-inl.h"
#include "monitor/monitor-inl.h"
#include "utils/list-inl.h"

namespace neutrino {

ref_scopeInfo ref_scope::current_;
RefBlock *ref_scope::spare_block_ = NULL;
list_buffer<RefBlock*> ref_scope::block_stack_;
Watch<HighWaterMark> ref_scope::block_count_("ref block count");

Value **ref_scope::grow() {
  ASSERT_C(NO_REF_SCOPE, current().block_count >= 0);
  RefBlock *extension;
  if (spare_block() == NULL) {
    block_count()->increment();
    extension = new RefBlock();
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

void ref_scope::shrink() {
  ASSERT(current().block_count != 0);
  uword blocks_to_delete = current().block_count;
  if (spare_block() == NULL) {
    spare_block_ = block_stack().pop();
    blocks_to_delete--;
  }
  for (uword i = 0; i < blocks_to_delete; i++) {
    block_count()->decrement();
    delete block_stack().pop();
  }
}

}
