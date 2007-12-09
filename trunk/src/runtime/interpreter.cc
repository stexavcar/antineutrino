#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "runtime/builtins-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/vector-inl.h"
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
  static inline void instruction(uint16_t opcode, Frame &frame);
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
  word *bottom = task->stack()->bottom() + Frame::accessible_below_fp(0);
  Frame frame(bottom, bottom + Frame::kSize);
  frame.self(0) = runtime().roots().vhoid();
  frame.lambda() = lambda;
  frame.prev_fp() = 0;
  uint32_t pc = 0;
  while (true) {
    Data *value = interpret(frame, &pc);
    if (is<Signal>(value)) {
      frame.push_activation();
      task->stack()->set_fp(frame.fp() - task->stack()->bottom());
      if (is<AllocationFailed>(value)) {
        Stack *old_stack = task_ref->stack();
        Runtime::current().heap().memory().collect_garbage();
        frame.reset(old_stack, task_ref->stack());
      } else {
        UNREACHABLE();
      }
      frame.unwind();
    } else {
      return cast<Value>(value);
    }
  }
}

void Frame::reset(Stack *old_stack, Stack *new_stack) {
  uint32_t delta = new_stack->bottom() - old_stack->bottom();
  fp_ += delta;
  sp_ += delta;
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

/**
 * A stack-allocated instance of this class ensures that execution
 * taking place on the specified stack is wrapped up correctly and
 * that the current execution status is stored in the task so
 * execution can continue afterwards.
 */
class ExecutionWrapUp {
public:
  inline ExecutionWrapUp(uint32_t *pc_from, uint32_t *pc_to) {
    pc_from_ = pc_from;
    pc_to_ = pc_to;
  }
  inline ~ExecutionWrapUp() {
    *pc_to_ = *pc_from_;
  }
private:
  uint32_t *pc_from_, *pc_to_;
};

Data *Interpreter::interpret(Frame &frame, uint32_t *pc_ptr) {
  uint32_t pc = *pc_ptr;
  ExecutionWrapUp wrap_up(&pc, pc_ptr);
  vector<uint16_t> code = cast<Code>(frame.lambda()->code())->buffer();
  vector<Value*> constant_pool = cast<Tuple>(frame.lambda()->literals())->buffer();
  while (true) {
    uint16_t oc = code[pc];
    Log::instruction(oc, frame);
    switch (oc) {
    case OC_PUSH: {
      uint16_t index = code[pc + 1];
      Value *value = constant_pool[index];
      frame.push(value);
      pc += OpcodeInfo<OC_PUSH>::kSize;
      break;
    }
    case OC_SLAP: {
      uint16_t height = code[pc + 1];
      frame[height] = frame[0];
      frame.pop(height);
      pc += OpcodeInfo<OC_SLAP>::kSize;
      break;
    }
    case OC_GLOBAL: {
      uint16_t index = code[pc + 1];
      Value *name = cast<Tuple>(frame.lambda()->literals())->at(index);
      Data *value = runtime().toplevel()->get(name);
      if (is<Nothing>(value)) {
        frame.push(runtime().roots().vhoid());
      } else {
        frame.push(cast<Value>(value));
      }
      pc += OpcodeInfo<OC_GLOBAL>::kSize;
      break;
    }
    case OC_LOCAL: {
      uint16_t index = code[pc + 1];
      Value *value = frame.local(index);
      frame.push(value);
      pc += OpcodeInfo<OC_LOCAL>::kSize;
      break;
    }
    case OC_ARGUMENT: {
      uint16_t index = code[pc + 1];
      Value *value = frame.argument(index);
      frame.push(value);
      pc += OpcodeInfo<OC_ARGUMENT>::kSize;
      break;
    }
    case OC_IF_TRUE: {
      Value *value = frame.pop();
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
      Value *name = cast<Tuple>(frame.lambda()->literals())->at(name_index);
      uint16_t argc = code[pc + 2];
      Value *recv = frame[argc + 1];
      Class *chlass = get_class(recv);
      Data *lookup_result = lookup_method(chlass, name);
      if (is<Nothing>(lookup_result)) {
        scoped_string name_str(name->to_string());
        scoped_string recv_str(recv->to_string());
        Conditions::get().error_occurred("Lookup failure: %s::%s",
            recv_str.chars(), name_str.chars());
      }
      Method *method = cast<Method>(lookup_result);
      frame.push_activation();
      frame.prev_pc() = pc + OpcodeInfo<OC_INVOKE>::kSize;
      frame.lambda() = method->lambda();
      new_ref(method->lambda()).ensure_compiled();
      code = cast<Code>(method->lambda()->code())->buffer();
      constant_pool = cast<Tuple>(method->lambda()->literals())->buffer();
      pc = 0;
      break;
    }
    case OC_CALL: {
      uint16_t argc = code[pc + 1];
      Value *value = frame[argc];
      Lambda *fun = cast<Lambda>(value);
      new_ref(fun).ensure_compiled();
      frame.push_activation();
      frame.prev_pc() = pc + OpcodeInfo<OC_CALL>::kSize;
      frame.lambda() = fun;
      code = cast<Code>(fun->code())->buffer();
      constant_pool = cast<Tuple>(fun->literals())->buffer();
      pc = 0;
      break;
    }
    case OC_BUILTIN: {
      uint16_t argc = code[pc + 1];
      uint16_t index = code[pc + 2];
      builtin *builtin = Builtins::get(index);
      Arguments args(runtime(), argc, frame);
      Value *value = cast<Value>(builtin(args));
      frame.push(value);
      pc += OpcodeInfo<OC_BUILTIN>::kSize;
      break;
    }
    case OC_RETURN: {
      Value *value = frame.pop();
      if (frame.is_bottom())
        return value;
      pc = frame.prev_pc();
      frame.unwind();
      code = cast<Code>(frame.lambda()->code())->buffer();
      constant_pool = cast<Tuple>(frame.lambda()->literals())->buffer();
      frame[0] = value;
      break;
    }
    case OC_VOID: {
      frame.push(runtime().roots().vhoid());
      pc += OpcodeInfo<OC_VOID>::kSize;
      break;
    }
    case OC_NULL: {
      frame.push(runtime().roots().nuhll());
      pc += OpcodeInfo<OC_NULL>::kSize;
      break;
    }
    case OC_TRUE: {
      frame.push(runtime().roots().thrue());
      pc += OpcodeInfo<OC_TRUE>::kSize;
      break;
    }
    case OC_FALSE: {
      frame.push(runtime().roots().fahlse());
      pc += OpcodeInfo<OC_FALSE>::kSize;
      break;
    }
    case OC_POP: {
      uint16_t height = code[pc + 1];
      frame.pop(height);
      pc += OpcodeInfo<OC_POP>::kSize;
      break;
    }
    case OC_TUPLE: {
      uint16_t length = code[pc + 1];
      Data *val = runtime().heap().new_tuple(length);
      if (is<Signal>(val)) return val;
      Tuple *result = cast<Tuple>(val);
      for (int32_t i = length - 1; i >= 0; i--)
        result->set(i, frame.pop());
      frame.push(result);
      pc += OpcodeInfo<OC_TUPLE>::kSize;
      break;
    }
    case OC_CHKHGT: {
      uint16_t expected = code[pc + 1];
      uint16_t height = (frame.sp() - frame.fp()) - Frame::kSize;
      CHECK_EQ(expected, height);
      pc += OpcodeInfo<OC_CHKHGT>::kSize;
      break;
    }
    case OC_CONCAT: {
      uint32_t terms = code[pc + 1];
      uint32_t length = 0;
      for (uint32_t i = 0; i < terms; i++)
        length += cast<String>(frame[i])->length();
      Data *val = runtime().heap().new_string(length);
      if (is<Signal>(val)) return val;
      String *result = cast<String>(val);
      uint32_t cursor = 0;
      for (int32_t i = terms - 1; i >= 0; i--) {
        String *term = cast<String>(frame[i]);
        for (uint32_t j = 0; j < term->length(); j++)
          result->set(cursor + j, term->at(j));
        cursor += term->length();
      }
      frame.pop(terms);
      frame.push(result);
      pc += OpcodeInfo<OC_CONCAT>::kSize;
      break;
    }
    case OC_CLOSURE: {
      uint32_t index = code[pc + 1];
      Lambda *lambda = cast<Lambda>(cast<Tuple>(frame.lambda()->literals())->at(index));
      uint32_t outer_count = cast<Code>(frame.lambda()->code())->at(pc + 2);
      Data *outers_val = runtime().heap().new_tuple(outer_count);
      if (is<Signal>(outers_val)) return outers_val;
      Tuple *outers = cast<Tuple>(outers_val);
      for (uint32_t i = 0; i < outer_count; i++)
        outers->set(outer_count - i - 1, frame.pop());
      Data *clone_val = lambda->clone(runtime().heap());
      if (is<Signal>(clone_val)) return clone_val;
      Lambda *clone = cast<Lambda>(clone_val);
      clone->set_outers(outers);
      frame.push(clone);
      pc += OpcodeInfo<OC_CLOSURE>::kSize;
      break;
    }
    case OC_OUTER: {
      uint32_t index = code[pc + 1];
      Value *value = frame.lambda()->outers()->at(index);
      frame.push(value);
      pc += OpcodeInfo<OC_OUTER>::kSize;
      break;
    }
    case OC_QUOTE: {
      uint32_t unquote_count = code[pc + 1];
      Data *unquotes_val = runtime().heap().new_tuple(unquote_count);
      if (is<Signal>(unquotes_val)) return unquotes_val;
      Tuple *unquotes = cast<Tuple>(unquotes_val);
      for (uint32_t i = 0; i < unquote_count; i++)
        unquotes->set(unquote_count - i - 1, frame.pop());
      SyntaxTree *tree = cast<SyntaxTree>(frame.pop());
      Data *result_val = runtime().heap().new_quote_template(tree, unquotes);
      if (is<Signal>(result_val)) return result_val;
      QuoteTemplate *result = cast<QuoteTemplate>(result_val);
      frame.push(result);
      pc += OpcodeInfo<OC_QUOTE>::kSize;
      break;
    }
    default:
      UNHANDLED(Opcode, oc);
      return Nothing::make();
    }
  }
}

static void print_stack(Stack *stack, word* offset = 0) {
  printf("--- stack ---\n");
  Frame current(stack->bottom() + stack->fp());
  while (!current.is_bottom()) {
    printf("--- frame ---\n");
    printf("sp:      %x\n", (int) current.sp());
    uint32_t stack_height = current.sp() - (current.fp() + Frame::kSize);
    printf("locals:\n");
#ifdef DEBUG
    EnumInfo<InstanceType> info;
    for (int32_t i = stack_height - 1; i >= 0; i--) {
      InstanceType type = current[i]->gc_safe_type();
      printf(" % i:  %s\n", stack_height - i, info.get_name_for(type).chars());
    }
#endif
    printf("fp:      %x\n", (int) current.fp());
    printf("prev fp: %x\n", (int) current.prev_fp());
    if (offset == 0) current.unwind();
    else current.unwind(offset);
  }
}

void Stack::validate_stack() {
}

void Stack::recook_stack() {
  word *bot = bottom();
  Frame frame(bot + fp());
  while (!frame.is_bottom()) {
    frame.prev_fp() = bot + reinterpret_cast<uint32_t>(frame.prev_fp());
    frame.unwind();
  }
}

void Stack::uncook_stack() {
  UncookedStackIterator iter(this);
  word *bot = bottom();
  while (!iter.at_end()) {
    // Create a copy of the frame so that we can unwind the original
    // frame and then do the uncooking
    IF_DEBUG(word *prev_fp = iter.frame().prev_fp());
    iter.frame().prev_fp() = reinterpret_cast<word*>(iter.frame().prev_fp() - bot);
    iter.advance();
    ASSERT(prev_fp == iter.frame().fp());
  }
}

void Stack::for_each_stack_field(FieldVisitor &visitor) {
  word *bot = bottom();
  Frame frame(bottom() + fp());
  while (!frame.is_bottom()) {
    uint32_t stack_height = frame.sp() - (frame.fp() + Frame::kSize);
    for (uint32_t i = 0; i < stack_height; i++) {
      // Visit the i'th local variable
      visitor.visit_field(&frame[i]);
    }
    frame.unwind(bot);
  }
}

void Log::instruction(uint16_t code, Frame &frame) {
#ifdef DEBUG
  if (kTraceInstructions) {
    EnumInfo<Opcode> info;
    string name = info.get_name_for(code);
    uint16_t height = (frame.sp() - frame.fp()) - Frame::kSize;
    printf("%s (%i)\n", name.chars(), height);
  }
#endif
}

} // namespace neutrino
