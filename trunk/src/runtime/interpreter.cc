#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "runtime/builtins-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"

namespace neutrino {

MAKE_ENUM_INFO_HEADER(Opcode)
#define MAKE_ENTRY(n, NAME, argc) MAKE_ENUM_INFO_ENTRY(OC_##NAME)
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

/**
 * Returns the class object for the given value.
 */
Class *Interpreter::get_class(Value *value) {
  if (is<Smi>(value)) return runtime().roots().smi_class();
  else return cast<Object>(value)->chlass();
}

/**
 * Returns the method with the specified name in the given class.  If
 * no method is found Nothing is returned.
 */
Data *Interpreter::lookup_method(Class *chlass, Value *name) {
  while (true) {
    ASSERT(!chlass->is_empty());
    Tuple *methods = chlass->methods();
    for (uint32_t i = 0; i < methods->length(); i++) {
      Method *method = cast<Method>(methods->at(i));
      if (method->name()->equals(name))
        return method;
    }
    Value *super = chlass->super();
    if (is<Void>(super)) break;
    chlass = cast<Class>(super);
  }
  return Nothing::make();
}

ref<Value> Interpreter::interpret(Stack &stack) {
  Frame current = stack.top();
  uint32_t pc = 0;
  while (true) {
    uint16_t oc = current.lambda()->code()->at(pc);
    Log::instruction(oc, stack);
    switch (oc) {
    case OC_PUSH: {
      uint16_t index = current.lambda()->code()->at(pc + 1);
      Value *value = current.lambda()->literals()->at(index);
      stack.push(value);
      pc += OpcodeInfo<OC_PUSH>::kSize;
      break;
    }
    case OC_SLAP: {
      uint16_t height = current.lambda()->code()->at(pc + 1);
      stack[height] = stack[0];
      stack.pop(height);
      pc += OpcodeInfo<OC_SLAP>::kSize;
      break;
    }
    case OC_GLOBAL: {
      uint16_t index = current.lambda()->code()->at(pc + 1);
      Value *name = current.lambda()->literals()->at(index);
      Data *value = runtime().toplevel()->get(name);
      if (is<Nothing>(value)) {
        stack.push(runtime().roots().vhoid());
      } else {
        stack.push(cast<Value>(value));
      }
      pc += OpcodeInfo<OC_GLOBAL>::kSize;
      break;
    }
    case OC_ARGUMENT: {
      uint16_t index = current.lambda()->code()->at(pc + 1);
      Value *value = stack.argument(index);
      stack.push(value);
      pc += OpcodeInfo<OC_ARGUMENT>::kSize;
      break;
    }
    case OC_IF_TRUE: {
      Value *value = stack.pop();
      if (is<True>(value)) {
        uint16_t index = current.lambda()->code()->at(pc + 1);
        pc = index;
      } else {
        pc += OpcodeInfo<OC_IF_TRUE>::kSize;
      }
      break;
    }
    case OC_GOTO: {
      uint16_t address = current.lambda()->code()->at(pc + 1);
      pc = address;
      break;
    }
    case OC_INVOKE: {
      uint16_t name_index = current.lambda()->code()->at(pc + 1);
      Value *name = current.lambda()->literals()->at(name_index);
      uint16_t argc = current.lambda()->code()->at(pc + 2);
      Value *recv = stack[argc + 1];
      Class *chlass = get_class(recv);
      Data *lookup_result = lookup_method(chlass, name);
      if (is<Nothing>(lookup_result)) {
        scoped_string name_str(name->to_string());
        scoped_string recv_str(recv->to_string());
        Conditions::get().error_occurred("Lookup failure: %s::%s",
            recv_str.chars(), name_str.chars());
      }
      Method *method = cast<Method>(lookup_result);
      Frame next = stack.push_activation();
      next.prev_pc() = pc + OpcodeInfo<OC_INVOKE>::kSize;
      next.lambda() = method->lambda();
      current = next;
      pc = 0;
      break;
    }
    case OC_CALL: {
      uint16_t argc = current.lambda()->code()->at(pc + 1);
      Value *value = stack[argc];
      Lambda *fun = cast<Lambda>(value);
      Frame next = stack.push_activation();
      next.prev_pc() = pc + OpcodeInfo<OC_CALL>::kSize;
      next.lambda() = fun;
      current = next;
      pc = 0;
      break;
    }
    case OC_INTERNAL: {
      uint16_t index = current.lambda()->code()->at(pc + 1);
      uint16_t argc = current.lambda()->code()->at(pc + 2);
      Builtin *builtin = Builtins::get(index);
      Arguments args(runtime(), argc, stack);
      Value *value = builtin(args);
      stack.push(value);
      pc += OpcodeInfo<OC_INTERNAL>::kSize;
      break;
    }
    case OC_RETURN: {
      Value *value = stack.pop();
      if (current.prev_fp() == 0)
        return new_ref(value);
      pc = current.prev_pc();
      current = stack.pop_activation();
      stack[0] = value;
      break;
    }
    case OC_VOID: {
      stack.push(runtime().roots().vhoid());
      pc += OpcodeInfo<OC_VOID>::kSize;
      break;
    }
    case OC_NULL: {
      stack.push(runtime().roots().nuhll());
      pc += OpcodeInfo<OC_NULL>::kSize;
      break;
    }
    case OC_TRUE: {
      stack.push(runtime().roots().thrue());
      pc += OpcodeInfo<OC_TRUE>::kSize;
      break;
    }
    case OC_FALSE: {
      stack.push(runtime().roots().fahlse());
      pc += OpcodeInfo<OC_FALSE>::kSize;
      break;
    }
    case OC_POP: {
      uint16_t height = current.lambda()->code()->at(pc + 1);
      stack.pop(height);
      pc += OpcodeInfo<OC_POP>::kSize;
      break;
    }
    case OC_TUPLE: {
      uint16_t length = current.lambda()->code()->at(pc + 1);
      Tuple *result = cast<Tuple>(runtime().heap().new_tuple(length));
      for (int32_t i = length - 1; i >= 0; i--)
        result->at(i) = stack.pop();
      stack.push(result);
      pc += OpcodeInfo<OC_TUPLE>::kSize;
      break;
    }
    default:
      UNHANDLED(Opcode, oc);
      return ref<Value>::empty();
    }
  }
}

void Log::instruction(uint16_t code, Stack &stack) {
#ifdef DEBUG
  if (kTraceInstructions) {
    EnumInfo<Opcode> info;
    string name = info.get_name_for(code);
    printf("%s (%i %i)\n", name.chars(), code, static_cast<int>(stack.sp() - stack.bottom()));
  }
#endif
}

} // namespace neutrino
