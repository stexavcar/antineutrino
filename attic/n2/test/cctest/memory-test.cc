#include "cctest/nunit-inl.h"
#include "heap/memory-inl.h"
#include "heap/ref-inl.h"
#include "heap/space.h"
#include "utils/checks.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(simple_migration) {
  LocalRuntime runtime;
  ref_block<> protect(runtime.refs());
  ref<Tuple> tuple = protect(runtime.factory().new_tuple(10).value());
  @check is<Tuple>(*tuple);
  Tuple *old_tuple = *tuple;
  Memory &memory = runtime.heap().memory();
  SemiSpace &old_space = memory.young_space();
  CHECK(old_space.contains(*tuple));
  memory.collect_garbage(runtime);
  SemiSpace &new_space = memory.young_space();
  CHECK(&old_space != &new_space);
  CHECK(*tuple != old_tuple);
  @check is<Tuple>(*tuple);
  CHECK(new_space.contains(*tuple));
}

TEST(garbage_removed) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Memory &memory = heap.memory();
  SemiSpace &old_space = memory.young_space();
  for (uword i = 0; i < 10; i++)
    heap.new_tuple(10);
  ref<True> value = runtime.thrue();
  @check is<True>(*value);
  CHECK(old_space.contains(*value));
  uword space_before = old_space.bytes_allocated();
  memory.collect_garbage(runtime);
  SemiSpace &new_space = memory.young_space();
  CHECK(&old_space != &new_space);
  uword space_after = new_space.bytes_allocated();
  @check is<True>(*value);
  CHECK(new_space.contains(*value));
  CHECK(space_before > space_after);
  memory.collect_garbage(runtime);
  uword space_after_after = memory.young_space().bytes_allocated();
  @check space_after_after == space_after;
}

TEST(migrate_cycle) {
  LocalRuntime runtime;
  ref_block<> protect(runtime.refs());
  Memory &memory = runtime.heap().memory();
  SemiSpace &old_space = memory.young_space();
  ref<Tuple> value = protect(runtime.factory().new_tuple(3).value());
  @check is<Tuple>(*value);
  value->set(0, *value);
  value->set(1, *value);
  value->set(2, *value);
  CHECK(old_space.contains(*value));
  memory.collect_garbage(runtime);
  @check is<Tuple>(*value);
  CHECK(value->get(0) == *value);
  CHECK(value->get(1) == *value);
  CHECK(value->get(2) == *value);
}

TEST(garbage_collector_monitor) {
  LocalRuntime runtime;
  Memory &memory = runtime.heap().memory();
  GarbageCollectionMonitor monitor(memory);
  CHECK(!monitor.has_collected_garbage());
  {
    GarbageCollectionMonitor nested_monitor(memory);
    CHECK(!monitor.has_collected_garbage());
    CHECK(!nested_monitor.has_collected_garbage());
    {
      GarbageCollectionMonitor double_nested_monitor(memory);
      CHECK(!monitor.has_collected_garbage());
      CHECK(!nested_monitor.has_collected_garbage());
      CHECK(!double_nested_monitor.has_collected_garbage());
    }
  }
  CHECK(!monitor.has_collected_garbage());
  memory.collect_garbage(runtime);
  CHECK(monitor.has_collected_garbage());
}

TEST(disallow_garbage_collection) {
  LocalRuntime runtime;
  Memory &memory = runtime.heap().memory();
  DisallowGarbageCollection disallow(memory);
  {
    DisallowGarbageCollection nested_disallow(memory);
    {
      DisallowGarbageCollection double_nested_disallow(memory);
    }
  }
  ASSERT_ABORTS(cnDisallowed, memory.collect_garbage(runtime));
}