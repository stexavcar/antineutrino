#include "heap/ref-inl.h"
#include "utils/list-inl.h"

namespace neutrino {

#ifdef MONITOR
static int32_t scope_count = 0;
static MonitoredVariable rsc_monitor("ref_scope_count", &scope_count);
static int32_t high_water_mark = 0;
static MonitoredVariable hwm_monitor("ref_scope_high_water_mark", &high_water_mark);
#endif // MONITOR

RefScopeInfo RefScope::current_;
RefBlock *RefScope::spare_block_ = NULL;
list_buffer<RefBlock*> RefScope::block_stack_;

Value **RefScope::grow() {
  ASSERT_C(NO_REF_SCOPE, current().block_count >= 0);
  RefBlock *extension;
  if (spare_block() == NULL) {
    extension = new RefBlock();
    IF_MONITOR(scope_count++);
    IF_MONITOR(high_water_mark = max(scope_count, high_water_mark));
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
    IF_MONITOR(scope_count--);
    delete block_stack().pop();
  }
}

}
