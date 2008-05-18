#ifndef _RUNTIME_INTERPRETER_INL
#define _RUNTIME_INTERPRETER_INL

#include "heap/pointer-inl.h"
#include "runtime/interpreter.h"
#include "utils/checks.h"

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

uword &StackState::prev_pc() {
  return reinterpret_cast<uword*>(fp_)[kPrevPcOffset];
}

word *&StackState::prev_fp() {
  return reinterpret_cast<word**>(fp_)[kPrevFpOffset];
}

Lambda *&StackState::lambda() {
  return reinterpret_cast<Lambda**>(fp_)[kLambdaOffset];
}

bool StackState::is_bottom() {
  return prev_fp() == 0;
}

Value *StackState::pop(uword height) {
  Value *result = *reinterpret_cast<Value**>(sp_ - 1);
  sp_ -= height;
  return result;
}

Value *&StackState::operator[](uword index) {
  return *reinterpret_cast<Value**>(sp_ - 1 - index);
}

Value *&StackState::argument(uword index) {
  return *reinterpret_cast<Value**>(fp_ - index - 1);
}

Value *&StackState::local(uword index) {
  return *reinterpret_cast<Value**>(fp_ + StackState::kSize + index);
}

Value *&StackState::self(uword argc) {
  return argument(argc);
}

void StackState::push(Value *value) {
  *(sp_++) = reinterpret_cast<word>(value);
}

Marker StackState::push_marker() {
  word *mp = sp_;
  sp_ += Marker::kSize;
  return Marker(mp);
}

Marker StackState::pop_marker() {
  return Marker(sp_ -= Marker::kSize);
}

void StackState::push_activation(uword prev_pc, Lambda *lambda) {
  word *prev_fp = fp_;
  fp_ = sp_;
  sp_ = fp_ + StackState::kSize;
  ASSERT(sp_ >= fp_ + kSize);
  this->prev_fp() = prev_fp;
  this->prev_pc() = prev_pc;
  this->lambda() = lambda;
}

void StackState::unwind() {
  sp_ = fp();
  fp_ = prev_fp();
}

void StackState::unwind(array<word> buffer) {
  sp_ = fp();
  fp_ = buffer.from_offset(reinterpret_cast<uword>(prev_fp()));
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
