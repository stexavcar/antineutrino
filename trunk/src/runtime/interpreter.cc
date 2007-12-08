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
  static inline void instruction(uint16_t opcode, StackBuffer &stack);
private:
  static const bool kTraceInstructions = false;
};

// -----------------------------
// --- I n t e r p r e t e r ---
// -----------------------------

Value *Interpreter::call(Lambda *lambda, Task *task) {
  RefScope scope;
  ref<Task> task_ref = new_ref(task);
  new_ref(lambda).ensure_compiled();
  StackBuffer stack(task->stack());
  stack.push(runtime().roots().vhoid()); // initial 'this'
  stack.push(runtime().roots().vhoid()); // initial lambda
  Frame top = stack.push_activation();
  top.lambda() = lambda;
  while (true) {
    Data *value = interpret(stack);
    if (is<Signal>(value)) {
      if (is<AllocationFailed>(value)) {
        Runtime::current().heap().memory().collect_garbage();
        stack.reset(task_ref->stack());
      } else {
        UNREACHABLE();
      }
    } else {
      return cast<Value>(value);
    }
  }
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

Data *Interpreter::interpret(StackBuffer &stack) {
  Frame current = stack.top();
  uint32_t pc = 0;
  uint16_t *code = cast<Code>(current.lambda()->code())->start();
  Value **constant_pool = cast<Tuple>(current.lambda()->literals())->start();
  while (true) {
    ASSERT_LT(pc, cast<Code>(current.lambda()->code())->length());
    uint16_t oc = code[pc];
    Log::instruction(oc, stack);
    switch (oc) {
    case OC_PUSH: {
      uint16_t index = code[pc + 1];
      Value *value = constant_pool[index];
      stack.push(value);
      pc += OpcodeInfo<OC_PUSH>::kSize;
      break;
    }
    case OC_SLAP: {
      uint16_t height = code[pc + 1];
      stack[height] = stack[0];
      stack.pop(height);
      pc += OpcodeInfo<OC_SLAP>::kSize;
      break;
    }
    case OC_GLOBAL: {
      uint16_t index = code[pc + 1];
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
      uint16_t index = code[pc + 1];
      Value *value = current.local(index);
      stack.push(value);
      pc += OpcodeInfo<OC_LOCAL>::kSize;
      break;
    }
    case OC_ARGUMENT: {
      uint16_t index = code[pc + 1];
      Value *value = current.argument(index);
      stack.push(value);
      pc += OpcodeInfo<OC_ARGUMENT>::kSize;
      break;
    }
    case OC_IF_TRUE: {
      Value *value = stack.pop();
      if (is<True>(value)) {
        uint16_t index = code[pc + 1];
        pc = index;
      } else {
        pc += OpcodeInfo<OC_IF_TRUE>::kSize;
      }
      break;
    }
    case OC_GOTO: {
      uint16_t address = code[pc + 1];
      pc = address;
      break;
    }
    case OC_INVOKE: {
      uint16_t name_index = code[pc + 1];
      Value *name = cast<Tuple>(current.lambda()->literals())->at(name_index);
      uint16_t argc = code[pc + 2];
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
      code = cast<Code>(method->lambda()->code())->start();
      constant_pool = cast<Tuple>(method->lambda()->literals())->start();
      current = next;
      pc = 0;
      break;
    }
    case OC_CALL: {
      uint16_t argc = code[pc + 1];
      Value *value = stack[argc];
      Lambda *fun = cast<Lambda>(value);
      new_ref(fun).ensure_compiled();
      Frame next = stack.push_activation();
      next.prev_pc() = pc + OpcodeInfo<OC_CALL>::kSize;
      next.lambda() = fun;
      code = cast<Code>(fun->code())->start();
      constant_pool = cast<Tuple>(fun->literals())->start();
      current = next;
      pc = 0;
      break;
    }
    case OC_BUILTIN: {
      uint16_t argc = code[pc + 1];
      uint16_t index = code[pc + 2];
      builtin *builtin = Builtins::get(index);
      Arguments args(runtime(), argc, current);
      Value *value = cast<Value>(builtin(args));
      stack.push(value);
      pc += OpcodeInfo<OC_BUILTIN>::kSize;
      break;
    }
    case OC_RETURN: {
      Value *value = stack.pop();
      if (current.prev_fp() == 0)
        return value;
      pc = current.prev_pc();
      current = stack.pop_activation();
      code = cast<Code>(current.lambda()->code())->start();
      constant_pool = cast<Tuple>(current.lambda()->literals())->start();
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
      uint16_t height = code[pc + 1];
      stack.pop(height);
      pc += OpcodeInfo<OC_POP>::kSize;
      break;
    }
    case OC_TUPLE: {
      uint16_t length = code[pc + 1];
      Data *val = runtime().heap().new_tuple(length);
      if (is<Signal>(val)) return val;
      Tuple *result = cast<Tuple>(val);
      for (int32_t i = length - 1; i >= 0; i--)
        result->set(i, stack.pop());
      stack.push(result);
      pc += OpcodeInfo<OC_TUPLE>::kSize;
      break;
    }
    case OC_CHKHGT: {
      uint16_t expected = code[pc + 1];
      uint16_t height = (stack.sp() - stack.fp()) - Frame::kSize;
      CHECK_EQ(expected, height);
      pc += OpcodeInfo<OC_CHKHGT>::kSize;
      break;
    }
    case OC_CONCAT: {
      uint32_t terms = code[pc + 1];
      uint32_t length = 0;
      for (uint32_t i = 0; i < terms; i++)
        length += cast<String>(stack[i])->length();
      Data *val = runtime().heap().new_string(length);
      if (is<Signal>(val)) return val;
      String *result = cast<String>(val);
      uint32_t cursor = 0;
      for (int32_t i = terms - 1; i >= 0; i--) {
        String *term = cast<String>(stack[i]);
        for (uint32_t j = 0; j < term->length(); j++)
          result->set(cursor + j, term->at(j));
        cursor += term->length();
      }
      stack.pop(terms);
      stack.push(result);
      pc += OpcodeInfo<OC_CONCAT>::kSize;
      break;
    }
    case OC_CLOSURE: {
      uint32_t index = code[pc + 1];
      Lambda *lambda = cast<Lambda>(cast<Tuple>(current.lambda()->literals())->at(index));
      uint32_t outer_count = cast<Code>(current.lambda()->code())->at(pc + 2);
      Data *outers_val = runtime().heap().new_tuple(outer_count);
      if (is<Signal>(outers_val)) return outers_val;
      Tuple *outers = cast<Tuple>(outers_val);
      for (uint32_t i = 0; i < outer_count; i++)
        outers->set(outer_count - i - 1, stack.pop());
      Data *clone_val = lambda->clone(runtime().heap());
      if (is<Signal>(clone_val)) return clone_val;
      Lambda *clone = cast<Lambda>(clone_val);
      clone->set_outers(outers);
      stack.push(clone);
      pc += OpcodeInfo<OC_CLOSURE>::kSize;
      break;
    }
    case OC_OUTER: {
      uint32_t index = code[pc + 1];
      Value *value = current.lambda()->outers()->at(index);
      stack.push(value);
      pc += OpcodeInfo<OC_OUTER>::kSize;
      break;
    }
    case OC_QUOTE: {
      uint32_t unquote_count = code[pc + 1];
      Data *unquotes_val = runtime().heap().new_tuple(unquote_count);
      if (is<Signal>(unquotes_val)) return unquotes_val;
      Tuple *unquotes = cast<Tuple>(unquotes_val);
      for (uint32_t i = 0; i < unquote_count; i++)
        unquotes->set(unquote_count - i - 1, stack.pop());
      SyntaxTree *tree = cast<SyntaxTree>(stack.pop());
      Data *result_val = runtime().heap().new_quote_template(tree, unquotes);
      if (is<Signal>(result_val)) return result_val;
      QuoteTemplate *result = cast<QuoteTemplate>(result_val);
      stack.push(result);
      pc += OpcodeInfo<OC_QUOTE>::kSize;
      break;
    }
    default:
      UNHANDLED(Opcode, oc);
      return Nothing::make();
    }
  }
}

void Stack::for_each_stack_field(FieldVisitor &visitor) {
  
}

void Log::instruction(uint16_t code, StackBuffer &stack) {
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
