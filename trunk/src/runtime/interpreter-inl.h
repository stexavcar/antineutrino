#ifndef _RUNTIME_INTERPRETER_INL
#define _RUNTIME_INTERPRETER_INL

#include "runtime/interpreter.h"
#include "utils/checks.h"

namespace neutrino {

// -----------------
// --- F r a m e ---
// -----------------

uint32_t &Frame::prev_pc() {
  return reinterpret_cast<uint32_t*>(fp_)[kPrevPcOffset];
}

word *&Frame::prev_fp() {
  return reinterpret_cast<word**>(fp_)[kPrevFpOffset];
}

Lambda *&Frame::lambda() {
  return reinterpret_cast<Lambda**>(fp_)[kLambdaOffset];
}

// -----------------
// --- S t a c k ---
// -----------------

Stack::Stack()
  : sp_(bottom())
  , fp_(0) { }

Value *Stack::pop(uint32_t height) {
  ASSERT(sp() > 0);
  Value *result = *reinterpret_cast<Value**>(sp_ - 1);
  sp_ -= height;
  return result;
}

Value *&Stack::operator[](uint32_t index) {
  ASSERT(sp() > bottom() + index);
  return *reinterpret_cast<Value**>(sp_ - 1 - index);
}

Value *&Stack::argument(uint32_t index) {
  return *reinterpret_cast<Value**>(fp_ - index - 1);
}

Value *&Stack::self(uint32_t argc) {
  return argument(argc + 1);
}


void Stack::push(Value *value) {
  ASSERT(sp() < bottom() + kLimit);
  *(sp_++) = reinterpret_cast<word>(value);
}

Frame Stack::push_activation() {
  Frame result(sp_);
  result.prev_fp() = fp_;
  fp_ = sp_;
  sp_ += Frame::kSize;
  return result;
}

Frame Stack::pop_activation() {
  Frame top = this->top();
  sp_ = top.fp();
  fp_ = top.prev_fp();
  return Frame(fp_);
}

// -----------------------------
// --- O p c o d e   I n f o ---
// -----------------------------

/**
 * For each bytecode there is a specialization of this class that
 * contains various properties of the bytecode.
 */
template <Opcode opcode>
class OpcodeInfo { };

// Generate specializations for the opcodes
#define DECLARE_OPCODE_INFO(n, NAME, argc)                           \
  template <> class OpcodeInfo<OC_##NAME> {                          \
  public:                                                            \
    static const uint32_t kArgc = argc;                              \
    static const uint32_t kSize = argc + 1;                          \
  };
FOR_EACH_OPCODE(DECLARE_OPCODE_INFO)
#undef DECLARE_OPCODE_INFO

} // namespace neutrino

#endif // _RUNTIME_INTERPRETER_INL
