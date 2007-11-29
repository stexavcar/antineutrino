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
  static inline void instruction(uint16_t opcode, OldStack &stack);
private:
  static const bool kTraceInstructions = false;
};

// -----------------------------
// --- I n t e r p r e t e r ---
// -----------------------------

ref<Value> Interpreter::call(ref<Lambda> lambda) {
  lambda.ensure_compiled();
  OldStack stack;
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
    if (chlass->is_empty()) {
      scoped_string chlass_str(chlass->name()->to_string());
      Conditions::get().error_occurred("Class %s is empty.", chlass_str.chars());
    }
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

ref<Value> Interpreter::interpret(OldStack &stack) {
  Frame current = stack.top();
  uint32_t pc = 0;
  while (true) {
    uint16_t oc = cast<Code>(current.lambda()->code())->at(pc);
    Log::instruction(oc, stack);
    switch (oc) {
    case OC_PUSH: {
      uint16_t index = cast<Code>(current.lambda()->code())->at(pc + 1);
      Value *value = cast<Tuple>(current.lambda()->literals())->at(index);
      stack.push(value);
      pc += OpcodeInfo<OC_PUSH>::kSize;
      break;
    }
    case OC_SLAP: {
      uint16_t height = cast<Code>(current.lambda()->code())->at(pc + 1);
      stack[height] = stack[0];
      stack.pop(height);
      pc += OpcodeInfo<OC_SLAP>::kSize;
      break;
    }
    case OC_GLOBAL: {
      uint16_t index = cast<Code>(current.lambda()->code())->at(pc + 1);
      Value *name = cast<Tuple>(current.lambda()->literals())->at(index);
      Data *value = runtime().toplevel()->get(name);
      if (is<Nothing>(value)) {
        stack.push(runtime().roots().vhoid());
      } else {
        stack.push(cast<Value>(value));
      }
      pc += OpcodeInfo<OC_GLOBAL>::kSize;
      break;
    }
    case OC_LOCAL: {
      uint16_t index = cast<Code>(current.lambda()->code())->at(pc + 1);
      Value *value = stack.local(index);
      stack.push(value);
      pc += OpcodeInfo<OC_LOCAL>::kSize;
      break;
    }
    case OC_ARGUMENT: {
      uint16_t index = cast<Code>(current.lambda()->code())->at(pc + 1);
      Value *value = stack.argument(index);
      stack.push(value);
      pc += OpcodeInfo<OC_ARGUMENT>::kSize;
      break;
    }
    case OC_IF_TRUE: {
      Value *value = stack.pop();
      if (is<True>(value)) {
        uint16_t index = cast<Code>(current.lambda()->code())->at(pc + 1);
        pc = index;
      } else {
        pc += OpcodeInfo<OC_IF_TRUE>::kSize;
      }
      break;
    }
    case OC_GOTO: {
      uint16_t address = cast<Code>(current.lambda()->code())->at(pc + 1);
      pc = address;
      break;
    }
    case OC_INVOKE: {
      uint16_t name_index = cast<Code>(current.lambda()->code())->at(pc + 1);
      Value *name = cast<Tuple>(current.lambda()->literals())->at(name_index);
      uint16_t argc = cast<Code>(current.lambda()->code())->at(pc + 2);
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
      new_ref(method->lambda()).ensure_compiled();
      current = next;
      pc = 0;
      break;
    }
    case OC_CALL: {
      uint16_t argc = cast<Code>(current.lambda()->code())->at(pc + 1);
      Value *value = stack[argc];
      Lambda *fun = cast<Lambda>(value);
      new_ref(fun).ensure_compiled();
      Frame next = stack.push_activation();
      next.prev_pc() = pc + OpcodeInfo<OC_CALL>::kSize;
      next.lambda() = fun;
      current = next;
      pc = 0;
      break;
    }
    case OC_BUILTIN: {
      uint16_t argc = cast<Code>(current.lambda()->code())->at(pc + 1);
      uint16_t index = cast<Code>(current.lambda()->code())->at(pc + 2);
      Builtin *builtin = Builtins::get(index);
      Arguments args(runtime(), argc, stack);
      Value *value = builtin(args);
      stack.push(value);
      pc += OpcodeInfo<OC_BUILTIN>::kSize;
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
      uint16_t height = cast<Code>(current.lambda()->code())->at(pc + 1);
      stack.pop(height);
      pc += OpcodeInfo<OC_POP>::kSize;
      break;
    }
    case OC_TUPLE: {
      RefScope scope;
      uint16_t length = cast<Code>(current.lambda()->code())->at(pc + 1);
      ref<Tuple> result = runtime().factory().new_tuple(length);
      for (int32_t i = length - 1; i >= 0; i--)
        result->at(i) = stack.pop();
      stack.push(*result);
      pc += OpcodeInfo<OC_TUPLE>::kSize;
      break;
    }
    case OC_CHKHGT: {
      uint16_t expected = cast<Code>(current.lambda()->code())->at(pc + 1);      
      uint16_t height = (stack.sp() - stack.fp()) - Frame::kSize;
      CHECK_EQ(expected, height);
      pc += OpcodeInfo<OC_CHKHGT>::kSize;
      break;
    }
    case OC_CONCAT: {
      RefScope scope;
      uint32_t terms = cast<Code>(current.lambda()->code())->at(pc + 1);
      uint32_t length = 0;
      for (uint32_t i = 0; i < terms; i++)
        length += cast<String>(stack[i])->length();
      ref<String> result = runtime().factory().new_string(length);
      uint32_t cursor = 0;
      for (int32_t i = terms - 1; i >= 0; i--) {
        String *term = cast<String>(stack[i]);
        for (uint32_t j = 0; j < term->length(); j++)
          result->set(cursor + j, term->at(j));
        cursor += term->length();
      }
      stack.pop(terms);
      stack.push(*result);
      pc += OpcodeInfo<OC_CONCAT>::kSize;
      break;
    }
    case OC_CLOSURE: {
      RefScope scope;
      uint32_t index = cast<Code>(current.lambda()->code())->at(pc + 1);
      ref<Lambda> lambda = new_ref(cast<Lambda>(cast<Tuple>(current.lambda()->literals())->at(index)));
      uint32_t outer_count = cast<Code>(current.lambda()->code())->at(pc + 2);
      ref<Tuple> outers = runtime().factory().new_tuple(outer_count);
      for (uint32_t i = 0; i < outer_count; i++)
        outers->set(outer_count - i - 1, stack.pop());
      ref<Lambda> clone = lambda.clone(runtime().factory());
      clone->set_outers(*outers);
      stack.push(*clone);
      pc += OpcodeInfo<OC_CLOSURE>::kSize;
      break;
    }
    case OC_OUTER: {
      uint32_t index = cast<Code>(current.lambda()->code())->at(pc + 1);
      Value *value = current.lambda()->outers()->at(index);
      stack.push(value);
      pc += OpcodeInfo<OC_OUTER>::kSize;
      break;
    }
    default:
      UNHANDLED(Opcode, oc);
      return ref<Value>::empty();
    }
  }
}

void Log::instruction(uint16_t code, OldStack &stack) {
#ifdef DEBUG
  if (kTraceInstructions) {
    EnumInfo<Opcode> info;
    string name = info.get_name_for(code);
    uint16_t height = (stack.sp() - stack.fp()) - Frame::kSize;
    printf("%s (%i)\n", name.chars(), height);
  }
#endif
}

} // namespace neutrino
