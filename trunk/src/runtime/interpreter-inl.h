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

StackBuffer::StackBuffer(Stack *stack)
  : height_(stack->height())
  , data_(stack->bottom())
  , sp_(data_)
  , fp_(0) { }

void StackBuffer::reset(Stack *stack) {
  uint32_t neutral_fp = fp_ - data_;
  uint32_t neutral_sp = sp_ - data_;
  height_ = stack->height();
  data_ = stack->bottom();
  fp_ = data_ + neutral_fp;
  sp_ = data_ + neutral_sp;
}

Value *StackBuffer::pop(uint32_t height) {
  ASSERT(sp() > 0);
  Value *result = *reinterpret_cast<Value**>(sp_ - 1);
  sp_ -= height;
  return result;
}

Value *&StackBuffer::operator[](uint32_t index) {
  ASSERT(sp() > bottom() + index);
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


void StackBuffer::push(Value *value) {
  ASSERT(sp() < limit());
  *(sp_++) = reinterpret_cast<word>(value);
}

Frame StackBuffer::push_activation() {
  Frame result(sp_);
  result.prev_fp() = fp_;
  fp_ = sp_;
  sp_ += Frame::kSize;
  return result;
}

Frame StackBuffer::pop_activation() {
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
