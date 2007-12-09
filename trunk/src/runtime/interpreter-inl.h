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

bool Frame::is_bottom() {
  return prev_fp() == 0;
}

// -----------------
// --- S t a c k ---
// -----------------

Value *Frame::pop(uint32_t height) {
  Value *result = *reinterpret_cast<Value**>(sp_ - 1);
  sp_ -= height;
  return result;
}

Value *&Frame::operator[](uint32_t index) {
  return *reinterpret_cast<Value**>(sp_ - 1 - index);
}

Value *&Frame::argument(uint32_t index) {
  return *reinterpret_cast<Value**>(fp_ - index - 1);
}

Value *&Frame::local(uint32_t index) {
  return *reinterpret_cast<Value**>(fp_ + Frame::kSize + index);
}

Value *&Frame::self(uint32_t argc) {
  return argument(argc + 1);
}

void Frame::push(Value *value) {
  *(sp_++) = reinterpret_cast<word>(value);
}

void Frame::push_activation() {
  word *prev_fp = fp_;
  fp_ = sp_;
  sp_ = fp_ + Frame::kSize;
  this->prev_fp() = prev_fp;
}

void Frame::unwind() {
  sp_ = fp();
  fp_ = prev_fp();
}

void Frame::unwind(word *bottom) {
  sp_ = fp();
  fp_ = reinterpret_cast<uint32_t>(prev_fp()) + bottom;
}

UncookedStackIterator::UncookedStackIterator(Stack *stack)
  : bottom_(stack->bottom())
  , frame_(bottom_ + stack->fp()) { }

void UncookedStackIterator::advance() {
  frame_.unwind(bottom_);
}

bool UncookedStackIterator::at_end() {
  return frame_.is_bottom();
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
