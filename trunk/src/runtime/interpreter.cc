#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"

namespace neutrino {

MAKE_ENUM_INFO_HEADER(Opcode)
#define MAKE_ENTRY(NAME, value, argc) MAKE_ENUM_INFO_ENTRY(NAME)
FOR_EACH_OPCODE(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

// -------------
// --- L o g ---
// -------------

class Log {
public:
  static inline void instruction(uint16_t opcode, Stack &stack);
private:
  static const bool kTraceInstructions = false;
};

// -----------------------------
// --- I n t e r p r e t e r ---
// -----------------------------

ref<Value> Interpreter::call(ref<Lambda> lambda) {
  Stack stack;
  stack.push(runtime().roots().vhoid()); // initial 'this'
  stack.push(runtime().roots().vhoid()); // initial lambda
  Frame top = stack.push_activation();
  top.lambda() = *lambda;
  return interpret(stack);
}

ref<Value> Interpreter::interpret(Stack &stack) {
  Frame current = stack.top();
  uint32_t pc = 0;
  while (true) {
    uint16_t oc = current.lambda()->code()->at(pc);
    Log::instruction(oc, stack);
    switch (oc) {
    case PUSH: {
      uint16_t index = current.lambda()->code()->at(pc + 1);
      Value *value = current.lambda()->literals()->at(index);
      stack.push(value);
      pc += OpcodeInfo<PUSH>::kSize;
      break;
    }
    case SLAP: {
      uint16_t height = current.lambda()->code()->at(pc + 1);
      stack[height] = stack[0];
      stack.pop(height);
      pc += OpcodeInfo<SLAP>::kSize;
      break;
    }
    case GLOBAL: {
      uint16_t index = current.lambda()->code()->at(pc + 1);
      Value *name = current.lambda()->literals()->at(index);
      Data *value = runtime().toplevel()->get(name);
      if (is<Nothing>(value)) {
        stack.push(runtime().roots().vhoid());
      } else {
        stack.push(cast<Value>(value));
      }
      pc += OpcodeInfo<GLOBAL>::kSize;
      break;
    }
    case ARGUMENT: {
      uint16_t index = current.lambda()->code()->at(pc + 1);
      Value *value = stack.argument(index);
      stack.push(value);
      pc += OpcodeInfo<ARGUMENT>::kSize;
      break;
    }
    case IF_TRUE: {
      Value *value = stack.pop();
      if (is<True>(value)) {
        uint16_t index = current.lambda()->code()->at(pc + 1);
        pc = index;
      } else {
        pc += OpcodeInfo<IF_TRUE>::kSize;
      }
      break;
    }
    case GOTO: {
      uint16_t address = current.lambda()->code()->at(pc + 1);
      pc = address;
      break;
    }
    case CALL: {
      uint16_t argc = current.lambda()->code()->at(pc + 1);
      Value *value = stack[argc];
      Lambda *fun = cast<Lambda>(value);
      Frame next = stack.push_activation();
      next.prev_pc() = pc + OpcodeInfo<CALL>::kSize;
      next.lambda() = fun;
      current = next;
      pc = 0;
      break;
    }
    case RETURN: {
      Value *value = stack.pop();
      if (current.prev_fp() == 0)
        return new_ref(value);
      pc = current.prev_pc();
      current = stack.pop_activation();
      stack[0] = value;
      break;
    }
    case VOID: {
      stack.push(runtime().roots().vhoid());
      pc += OpcodeInfo<VOID>::kSize;
      break;
    }
    case NUHLL: {
      stack.push(runtime().roots().nuhll());
      pc += OpcodeInfo<NUHLL>::kSize;
      break;
    }
    case TRUE: {
      stack.push(runtime().roots().thrue());
      pc += OpcodeInfo<TRUE>::kSize;
      break;
    }
    case FALSE: {
      stack.push(runtime().roots().fahlse());
      pc += OpcodeInfo<FALSE>::kSize;
      break;
    }
    default:
      UNHANDLED(Opcode, oc);
      return ref<Value>::empty();
    }
  }
}

void Log::instruction(uint16_t code, Stack &stack) {
  if (kTraceInstructions) {
    EnumInfo<Opcode> info;
    string name = info.get_name_for(code);
    printf("%s (%i)\n", name.chars(), static_cast<int>(stack.sp() - stack.bottom()));
  }
}

} // namespace neutrino
