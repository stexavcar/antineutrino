#include "value/pointer-inl.h"
#include "value/value-inl.h"
#include "test-inl.h"

using namespace neutrino;

TEST(signal) {
  InternalError *ie = InternalError::unknown();
  assert is<Signal>(ie);
  assert is<InternalError>(ie);
  assert (!is<Success>(ie));
  assert ie->payload() == InternalError::ieUnknown;
  Success *success = Success::make();
  assert is<Signal>(success);
  assert is<Success>(success);
  assert (!is<InternalError>(success));
}
