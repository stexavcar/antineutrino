#include "cctest/tests-inl.h"
#include "heap/memory-inl.h"
#include "heap/ref-inl.h"
#include "heap/space.h"
#include "utils/checks.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::simple_migration() {
  LocalRuntime runtime;
  RefScope scope;
  ref<Tuple> tuple = runtime.factory().new_tuple(10);
  CHECK_IS(Tuple, *tuple);
  Tuple *old_tuple = *tuple;
  Memory &memory = runtime.heap().memory();
  SemiSpace &old_space = memory.young_space();
  CHECK(old_space.contains(*tuple));
  memory.collect_garbage();
  SemiSpace &new_space = memory.young_space();
  CHECK(&old_space != &new_space);
  CHECK(*tuple != old_tuple);
  CHECK_IS(Tuple, *tuple);
  CHECK(new_space.contains(*tuple));
}

void Test::garbage_removed() {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Memory &memory = heap.memory();
  SemiSpace &old_space = memory.young_space();
  for (uint32_t i = 0; i < 10; i++)
    heap.new_tuple(10);
  RefScope scope;
  ref<True> value = runtime.thrue();
  CHECK_IS(True, *value);
  CHECK(old_space.contains(*value));
  uint32_t space_before = old_space.bytes_allocated();
  memory.collect_garbage();
  SemiSpace &new_space = memory.young_space();
  CHECK(&old_space != &new_space);
  uint32_t space_after = new_space.bytes_allocated();
  CHECK_IS(True, *value);
  CHECK(new_space.contains(*value));
  CHECK(space_before > space_after);
  memory.collect_garbage();
  uint32_t space_after_after = memory.young_space().bytes_allocated();
  CHECK_EQ(space_after, space_after_after);
}

void Test::migrate_cycle() {
  LocalRuntime runtime;
  RefScope scope;
  Memory &memory = runtime.heap().memory();
  Factory &factory = runtime.factory();
  SemiSpace &old_space = memory.young_space();
  ref<Tuple> value = factory.new_tuple(3);
  CHECK_IS(Tuple, *value);
  value->set(0, *value);
  value->set(1, *value);
  value->set(2, *value);
  CHECK(old_space.contains(*value));
  memory.collect_garbage();
  CHECK_IS(Tuple, *value);
  CHECK(value->get(0) == *value);
  CHECK(value->get(1) == *value);
  CHECK(value->get(2) == *value);
}

void Test::garbage_collector_monitor() {
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
  memory.collect_garbage();
  CHECK(monitor.has_collected_garbage());
}

void Test::disallow_garbage_collection() {
  LocalRuntime runtime;
  Memory &memory = runtime.heap().memory();
  DisallowGarbageCollection disallow(memory);
  {
    DisallowGarbageCollection nested_disallow(memory);
    {
      DisallowGarbageCollection double_nested_disallow(memory);
    }
  }
  ASSERT_ABORTS(DISALLOWED, memory.collect_garbage());
}
