#include "heap/ref-inl.h"
#include "utils/list-inl.h"

namespace neutrino {

int32_t RefScope::ref_scope_count_ = 0;
static MonitoredVariable rsc_monitor("ref_scope_count", &RefScope::ref_scope_count_);

int32_t RefScope::ref_scope_high_water_mark_ = 0;
static MonitoredVariable rshwm_monitor("ref_scope_high_water_mark", &RefScope::ref_scope_high_water_mark_);

RefScopeInfo RefScope::current_;
RefBlock *RefScope::spare_block_ = NULL;
list_buffer<RefBlock*> RefScope::block_stack_;

Value **RefScope::grow() {
  ASSERT_C(NO_REF_SCOPE, current().block_count >= 0);
  RefBlock *extension;
  if (spare_block() == NULL) {
    extension = new RefBlock();
    ref_scope_count_++;
    ref_scope_high_water_mark_ = max(ref_scope_count_, ref_scope_high_water_mark_);
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

void RefScope::shrink() {
  ASSERT(current().block_count != 0);
  uint32_t blocks_to_delete = current().block_count;
  if (spare_block() == NULL) {
    spare_block_ = block_stack().pop();
    blocks_to_delete--;
  }
  for (uint32_t i = 0; i < blocks_to_delete; i++) {
    ref_scope_count_--;
    delete block_stack().pop();
  }
}

}
