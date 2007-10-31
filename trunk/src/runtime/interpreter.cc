#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"

namespace neutrino {

MAKE_ENUM_INFO_HEADER(Opcode)
#define MAKE_ENTRY(NAME, Name) MAKE_ENUM_INFO_ENTRY(NAME)
FOR_EACH_OPCODE(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

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
  : sp_(0)
  , fp_(data_) { }

Value *Stack::pop_value() {
  ASSERT(sp() > 0);
  return reinterpret_cast<Value*>(data()[--sp_]);
}

Value *Stack::peek_value() {
  ASSERT(sp() > 0);
  return reinterpret_cast<Value*>(data()[sp_ - 1]);
}

void Stack::push_value(Value *value) {
  push_word(reinterpret_cast<word>(value));
}

void Stack::push_word(word value) {
  ASSERT(sp() < kLimit);
  data()[sp_++] = value;
}

Frame Stack::push_activation() {
  Frame result(data_ + sp_);
  result.prev_fp() = fp_;
  fp_ = data_ + sp_;
  sp_ += Frame::kSize;
  return result;
}

// -----------------------------
// --- I n t e r p r e t e r ---
// -----------------------------

ref<Value> Interpreter::call(ref<Lambda> lambda) {
  Stack stack;
  Frame top = stack.push_activation();
  top.lambda() = *lambda;
  return interpret(stack);
}

ref<Value> Interpreter::interpret(Stack &stack) {
  Frame top = stack.top();
  Code *code = top.lambda()->code();
  Tuple *literals = top.lambda()->literals();
  uint32_t pc = 0;
  while (true) {
    switch (code->at(pc)) {
    case PUSH: {
      uint16_t index = code->at(pc + 1);
      Value *value = literals->at(index);
      stack.push_value(value);
      pc += 2;
      break;
    }
    case GLOBAL: {
      uint16_t index = code->at(pc + 1);
      Value *name = literals->at(index);
      Data *value = Runtime::current().toplevel()->get(name);
      if (is<Nothing>(value)) {
        stack.push_value(Runtime::current().roots().vhoid());
      } else {
        stack.push_value(cast<Value>(value));
      }
      pc += 2;
      break;
    }
    case CALL: {
      Value *value = stack.peek_value();
      Lambda *fun = cast<Lambda>(value);
      stack.push_value(top.lambda());
      stack.push_word(pc + 2);
      code = fun->code();
      literals = fun->literals();
      pc = 0;
      break;
    }
    case RETURN: {
      return new_ref(stack.pop_value());
    }
    default:
      UNHANDLED(Opcode, code->at(pc));
      return ref<Value>::empty();
    }
  }
}

}
