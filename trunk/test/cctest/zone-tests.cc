#include "heap/zone-inl.h"

using namespace neutrino::zone;

static void test_string() {
  Zone zone;
  for (int i = 0; i < 100; i++)
    zone.allocate(Arena::kSize / 4);
}

static void test_list() {
  Zone zone;
  for (int i = 0; i < 1000; i++) {
    Vector<uint32_t> &vector = *(new Vector<uint32_t>(10));
    for (int i = 0; i < 10; i++) vector[i] = 5 * i;
    for (int i = 0; i < 10; i++) ASSERT_EQ(vector[i], 5 * i);
  }
  CHECK(zone.bytes_allocated() >= 1000 * (sizeof(Vector<uint32_t>) + 10 * sizeof(uint32_t)));
}
