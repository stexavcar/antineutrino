#ifndef _RUNTIME_INTERPRETER_INL
#define _RUNTIME_INTERPRETER_INL

#include "heap/pointer-inl.h"
#include "runtime/interpreter.h"
#include "utils/checks.h"
#include "utils/smart-ptrs-inl.h"

namespace neutrino {


// -------------------
// --- M a r k e r ---
// -------------------

void Marker::set_prev_mp(word *value) {
  // the value must be smi tagged so it doesn't get mistaken for a
  // real object by the gc
  ASSERT(ValuePointer::has_smi_tag(value));
  mp()[kPrevMpOffset] = reinterpret_cast<word>(value);
}

word *Marker::prev_mp() {
  return reinterpret_cast<word*>(mp()[kPrevMpOffset]);
}

void Marker::set_data(Value *data) {
  mp()[kDataOffset] = reinterpret_cast<word>(data);
}

Value *Marker::data() {
  return reinterpret_cast<Value*>(mp()[kDataOffset]);
}

bool Marker::is_bottom() {
  return mp() == NULL;
}

void Marker::unwind() {
  mp_ = prev_mp();
}


// -----------------
// --- F r a m e ---
// -----------------

uword StackState::unpark(Stack *stack) {
  ASSERT_EQ(Stack::Status::ssParked, stack->status().state);
  IF_DEBUG(stack->status().state = Stack::Status::ssRunning);
  fp_ = stack->bound(stack->fp());
  sp_ = stack->bound(stack->sp());
  return stack->pc();
}

uword &StackState::prev_pc() {
  return reinterpret_cast<uword&>(fp_[kPrevPcOffset]);
}

word *&StackState::prev_fp() {
  return reinterpret_cast<word*&>(fp_[kPrevFpOffset]);
}

Lambda *&StackState::lambda() {
  return reinterpret_cast<Lambda*&>(fp_[kLambdaOffset]);
}

bool StackState::is_bottom() {
  return prev_fp() == 0;
}

Value *StackState::pop(uword height) {
  Value *result = reinterpret_cast<Value*>(*(sp_ - 1));
  sp_ = sp_ - height;
  return result;
}

Value *&StackState::operator[](uword index) {
  return reinterpret_cast<Value*&>(*(sp_ - 1 - index));
}

Value *&StackState::argument(uword index) {
  return reinterpret_cast<Value*&>(*(fp_ - index - 1));
}

Value *&StackState::local(uword index) {
  return reinterpret_cast<Value*&>(*(fp_ + StackState::kSize + index));
}

Value *&StackState::self(uword argc) {
  return argument(argc);
}

void StackState::push(Value *value) {
  *sp_ = reinterpret_cast<word>(value);
  sp_ = sp_ + 1;
}

Marker StackState::push_marker() {
  bounded_ptr<word> mp = sp_;
  sp_ = sp_ + Marker::kSize;
  return Marker(mp.value());
}

Marker StackState::pop_marker() {
  sp_ = sp_ - Marker::kSize;
  return Marker(sp_.value());
}

void StackState::push_activation(uword prev_pc, Lambda *lambda) {
  bounded_ptr<word> prev_fp = fp_;
  fp_ = sp_;
  sp_ = fp_ + StackState::kSize;
  ASSERT(sp_ >= fp_ + kSize);
  this->prev_fp() = prev_fp.value();
  this->prev_pc() = prev_pc;
  this->lambda() = lambda;
}

void StackState::unwind(Stack *stack) {
  sp_ = fp();
  fp_ = stack->bound(prev_fp());
}

void StackState::unwind(array<word> buffer, uword height) {
  sp_ = fp();
  word *raw_ptr = buffer.from_offset(reinterpret_cast<uword>(prev_fp()));
  fp_ = NEW_BOUNDED_PTR(raw_ptr, buffer.start(), buffer.start() + height);
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
#define DECLARE_OPCODE_INFO(n, Name, argc, argf)                     \
  template <> class OpcodeInfo<oc##Name> {                           \
  public:                                                            \
    static const uword kArgc = argc;                                 \
    static const uword kSize = argc + 1;                             \
  };
eOpcodes(DECLARE_OPCODE_INFO)
#undef DECLARE_OPCODE_INFO

} // namespace neutrino

#endif // _RUNTIME_INTERPRETER_INL
