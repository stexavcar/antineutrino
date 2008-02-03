#include "heap/ref-inl.h"
#include "runtime/builtins-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/vector-inl.h"
#include "utils/checks.h"
#include "values/values-inl.h"

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

Value *Interpreter::call(Lambda *initial_lambda, Task *initial_task) {
  RefScope scope;
  ref<Task> task = new_ref(initial_task);
  ref<Lambda> lambda = new_ref(initial_lambda);
  lambda.ensure_compiled();
  ASSERT(task->stack()->status().is_empty);
  task->stack()->status().is_empty = false;
  uword argc = 0;
  Frame frame(task->stack()->bottom() + Frame::accessible_below_fp(argc));
  frame.self(argc) = runtime().roots().vhoid();
  frame.lambda() = *lambda;
  frame.prev_fp() = 0;
  ASSERT(frame.sp() >= frame.fp() + Frame::kSize);
  uword pc = 0;
  Data *value;
  while (true) {
    value = interpret(task->stack(), frame, &pc);
    if (is<Signal>(value)) {
      frame.push_activation();
      task->stack()->set_fp(frame.fp() - task->stack()->bottom());
      frame.prev_pc() = pc;
      frame.lambda() = *lambda;
      IF_DEBUG(task->stack()->status().is_parked = true);
      if (is<AllocationFailed>(value)) {
        Stack *old_stack = task->stack();
        Runtime::current().heap().memory().collect_garbage();
        frame.reset(old_stack, task->stack());
      } else {
        UNREACHABLE();
      }
      IF_DEBUG(task->stack()->status().is_parked = false);
      frame.unwind();
    } else {
      break;
    }
  }
  task->stack()->status().is_empty = true;
  return cast<Value>(value);
}

void Frame::reset(Stack *old_stack, Stack *new_stack) {
  uword delta = new_stack->bottom() - old_stack->bottom();
  fp_ += delta;
  sp_ += delta;
}

/**
 * Returns the class object for the given value.
 */
Layout *Interpreter::get_layout(Immediate *value) {
  if (is<Smi>(value)) return runtime().roots().smi_layout();
  else return cast<Object>(value)->layout();
}


class MethodLookup {
public:
  MethodLookup()
    : score_(static_cast<uword>(-1))
    , method_(Nothing::make())
    , is_ambiguous_(false) { }
  void score_method(Method *method, Layout *reciever);
  void lookup_in_dictionary(Tuple *methods, Immediate *name, Layout *reciever);
  Data *method() { return method_; }
  bool is_ambiguous() { return is_ambiguous_; }
private:
  uword score_;
  Data *method_;
  bool is_ambiguous_;
};


static inline word get_distance(Layout *layout, Protocol *target) {
  Value *current = layout->protocol();
  word distance = 0;
  while (is<Protocol>(current)) {
    if (current == target) return distance;
    distance++;
    current = cast<Protocol>(current)->super();
  }
  return -1;
}


void MethodLookup::score_method(Method *method, Layout *reciever) {
  uword score = 0;
  Signature *signature = method->signature();
  for (uword i = 0; i < signature->parameters()->length(); i++) {
    Value *param = signature->parameters()->get(i);
    if (is<Protocol>(param)) {
      word distance = get_distance(reciever, cast<Protocol>(param));
      if (distance >= 0) score += distance;
      else return;
    }
  }
  if (score < score_) {
    score_ = score;
    method_ = method;
    is_ambiguous_ = false;
  } else if (score == score_) {
    is_ambiguous_ = true;
  }
}


void MethodLookup::lookup_in_dictionary(Tuple *methods, Immediate *name,
    Layout *reciever) {
  for (uword i = 0; i < methods->length(); i++) {
    Method *method = cast<Method>(methods->get(i));
    if (method->name()->equals(name))
      score_method(method, reciever);
  }
}


/**
 * Returns the method with the specified name in the given class.  If
 * no method is found Nothing is returned.
 */
Data *Interpreter::lookup_method(Layout *layout, Immediate *name) {
  // Look up any layout-local methods
  MethodLookup lookup;
  lookup.lookup_in_dictionary(layout->methods(), name, layout);
  // Look up methods in protocols
  Value *current = layout->protocol();
  while (is<Protocol>(current)) {
    Protocol *protocol = cast<Protocol>(current);
    lookup.lookup_in_dictionary(protocol->methods(), name, layout);
    current = protocol->super();
  }
  ASSERT(!lookup.is_ambiguous());
  return lookup.method();
}

static void unhandled_condition(Value *name, Arguments &args) {
  string_buffer buf;
  buf.append("Unhandled condition: ");
  name->write_on(buf, Data::UNQUOTED);
  buf.append("(");
  bool is_first = true;
  for (uword i = 0; i < args.count(); i++) {
    if (is_first) is_first = false;
    else buf.append(", ");
    args[i]->write_on(buf);
  }
  buf.append(")");
  scoped_string str(buf.to_string());
  fprintf(stderr, "%s\n", str.chars());
  exit(1);
}

/**
 * A stack-allocated instance of this class ensures that execution
 * taking place on the specified stack is wrapped up correctly and
 * that the current execution status is stored in the task so
 * execution can continue afterwards.
 */
class ExecutionWrapUp {
public:
  inline ExecutionWrapUp(uword *pc_from, uword *pc_to) {
    pc_from_ = pc_from;
    pc_to_ = pc_to;
  }
  inline ~ExecutionWrapUp() {
    *pc_to_ = *pc_from_;
  }
private:
  uword *pc_from_, *pc_to_;
};

Data *Interpreter::interpret(Stack *stack, Frame &frame, uword *pc_ptr) {
  uword pc = *pc_ptr;
  ExecutionWrapUp wrap_up(&pc, pc_ptr);
  vector<uint16_t> code = cast<Code>(frame.lambda()->code())->buffer();
  vector<Value*> constant_pool = cast<Tuple>(frame.lambda()->constant_pool())->buffer();
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
      Value *name = constant_pool[index];
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
      Immediate *name = cast<Immediate>(constant_pool[name_index]);
      uint16_t argc = code[pc + 2];
      Value *recv = frame[argc + 1];
      Layout *layout = get_layout(deref(recv));
      Data *lookup_result = lookup_method(layout, name);
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
      method->lambda()->ensure_compiled();
      code = cast<Code>(method->lambda()->code())->buffer();
      constant_pool = cast<Tuple>(method->lambda()->constant_pool())->buffer();
      pc = 0;
      break;
    }
    case OC_RAISE: {
      uint16_t name_index = code[pc + 1];
      Value *name = constant_pool[name_index];
      uint16_t argc = code[pc + 2];
      Marker marker(stack->top_marker());
      while (!marker.is_bottom()) {
        Tuple *handlers = cast<Tuple>(marker.data());
        for (uword i = 0; i < handlers->length(); i += 2) {
          String *handler = cast<String>(handlers->get(i));
          if (name->equals(handler)) {
            Lambda *lambda = cast<Lambda>(handlers->get(i + 1));
            lambda->ensure_compiled();
            frame.push_activation();
            frame.prev_pc() = pc + OpcodeInfo<OC_RAISE>::kSize;
            frame.lambda() = lambda;
            code = cast<Code>(lambda->code())->buffer();
            constant_pool = cast<Tuple>(lambda->constant_pool())->buffer();
            pc = 0;
            goto end;
          }
        }
        marker.unwind();
      }
      {
        Arguments args(runtime(), argc, frame);
        unhandled_condition(name, args);
        pc += OpcodeInfo<OC_RAISE>::kSize;
      }
     end:
      break;
    }
    case OC_CALL: {
      uint16_t argc = code[pc + 1];
      Value *value = frame[argc];
      Lambda *fun = cast<Lambda>(value);
      fun->ensure_compiled();
      frame.push_activation();
      frame.prev_pc() = pc + OpcodeInfo<OC_CALL>::kSize;
      frame.lambda() = fun;
      code = cast<Code>(fun->code())->buffer();
      constant_pool = cast<Tuple>(fun->constant_pool())->buffer();
      pc = 0;
      break;
    }
    case OC_NEW: {
      uint16_t layout_template_index = code[pc + 1];
      Layout *layout_template = cast<Layout>(constant_pool[layout_template_index]);
      uword field_count = layout_template->instance_field_count();
      Protocol *proto = cast<Protocol>(frame[field_count]);
      Data *layout_val = layout_template->clone(runtime().heap());
      if (is<AllocationFailed>(layout_val)) return layout_val;
      Layout *layout = cast<Layout>(layout_val);
      layout->set_protocol(proto);
      Data *val = runtime().heap().new_instance(layout);
      if (is<Signal>(val)) return val;
      Instance *instance = cast<Instance>(val);
      for (word i = field_count - 1; i >= 0; i--)
        instance->set_field(i, frame.pop());
      frame.pop(); // Pop the protocol
      frame.push(instance);
      pc += OpcodeInfo<OC_NEW>::kSize;
      break;
    }
    case OC_FIELD: {
      uint16_t index = code[pc + 1];
      uint16_t argc = code[pc + 2];
      Value *value = cast<Instance>(to<Instance>(frame.self(argc)))->get_field(index);
      frame.push(value);
      pc += OpcodeInfo<OC_FIELD>::kSize;
      break;
    }
    case OC_MARK: {
      uint16_t data_index = code[pc + 1];
      Value *data = constant_pool[data_index];
      Marker mark = frame.push_marker();
      mark.set_data(data);
      word *current_marker = stack->top_marker();
      mark.set_prev_mp(current_marker);
      stack->set_top_marker(mark.mp());
      pc += OpcodeInfo<OC_MARK>::kSize;
      break;
    }
    case OC_UNMARK: {
      Value *value = frame.pop();
      Marker mark = frame.pop_marker();
      word *marker_value = mark.prev_mp();
      ASSERT(ValuePointer::has_smi_tag(marker_value));
      stack->set_top_marker(marker_value);
      frame.push(value);
      pc += OpcodeInfo<OC_UNMARK>::kSize;
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
      constant_pool = cast<Tuple>(frame.lambda()->constant_pool())->buffer();
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
      for (word i = length - 1; i >= 0; i--)
        result->set(i, frame.pop());
      frame.push(result);
      pc += OpcodeInfo<OC_TUPLE>::kSize;
      break;
    }
    case OC_CHKHGT: {
      uint16_t expected = code[pc + 1];
      CHECK_EQ(expected, frame.locals());
      pc += OpcodeInfo<OC_CHKHGT>::kSize;
      break;
    }
    case OC_CONCAT: {
      uword terms = code[pc + 1];
      uword length = 0;
      for (uword i = 0; i < terms; i++)
        length += cast<String>(frame[i])->length();
      Data *val = runtime().heap().new_string(length);
      if (is<Signal>(val)) return val;
      String *result = cast<String>(val);
      uword cursor = 0;
      for (word i = terms - 1; i >= 0; i--) {
        String *term = cast<String>(frame[i]);
        for (uword j = 0; j < term->length(); j++)
          result->set(cursor + j, term->at(j));
        cursor += term->length();
      }
      frame.pop(terms);
      frame.push(result);
      pc += OpcodeInfo<OC_CONCAT>::kSize;
      break;
    }
    case OC_CLOSURE: {
      uword index = code[pc + 1];
      Lambda *lambda = cast<Lambda>(constant_pool[index]);
      uword outer_count = cast<Code>(frame.lambda()->code())->at(pc + 2);
      Data *outers_val = runtime().heap().new_tuple(outer_count);
      if (is<Signal>(outers_val)) return outers_val;
      Tuple *outers = cast<Tuple>(outers_val);
      for (uword i = 0; i < outer_count; i++)
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
      uword index = code[pc + 1];
      Value *value = frame.lambda()->outers()->get(index);
      frame.push(value);
      pc += OpcodeInfo<OC_OUTER>::kSize;
      break;
    }
    case OC_QUOTE: {
      uword unquote_count = code[pc + 1];
      Data *unquotes_val = runtime().heap().new_tuple(unquote_count);
      if (is<Signal>(unquotes_val)) return unquotes_val;
      Tuple *unquotes = cast<Tuple>(unquotes_val);
      for (uword i = 0; i < unquote_count; i++)
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

#ifdef DEBUG
void Stack::validate_stack() {
  if (status().is_empty) return;
  ASSERT(status().is_cooked);
  Frame frame(bottom() + fp());
  while (true) {
    GC_SAFE_CHECK_IS_C(VALIDATION, Lambda, frame.lambda());
    if (frame.is_bottom()) break;
    uword local_count = frame.locals();
    for (uword i = 0; i < local_count; i++)
      GC_SAFE_CHECK_IS_C(VALIDATION, Value, frame[i]);
    frame.unwind();
  }
}
#endif

void Stack::recook_stack() {
  if (status().is_empty) return;
  ASSERT(!status().is_cooked);
  ASSERT(status().is_parked);
  word *bot = bottom();
  Frame frame(bot + fp());
  while (!frame.is_bottom()) {
    frame.prev_fp() = bot + reinterpret_cast<uword>(frame.prev_fp());
    frame.unwind();
  }
  IF_DEBUG(status().is_cooked = true);
}

void Stack::uncook_stack() {
  if (status().is_empty) return;
  ASSERT(status().is_cooked);
  ASSERT(status().is_parked);
  Frame frame(bottom() + fp());
  word *bot = bottom();
  while (!frame.is_bottom()) {
    // Create a copy of the frame so that we can unwind the original
    // frame and then do the uncooking
    IF_DEBUG(word *prev_fp = frame.prev_fp());
    frame.prev_fp() = reinterpret_cast<word*>(frame.prev_fp() - bot);
    frame.unwind(bot);
    ASSERT(prev_fp == frame.fp());
  }
  IF_DEBUG(status().is_cooked = false);
}

void Stack::for_each_stack_field(FieldVisitor &visitor) {
  if (status().is_empty) return;
  ASSERT(!status().is_cooked);
  ASSERT(status().is_parked);
  word *bot = bottom();
  Frame frame(bottom() + fp());
  while (true) {
    visitor.visit_field(pointer_cast<Value*>(&frame.lambda()));
    if (frame.is_bottom()) break;
    uword stack_height = frame.locals();
    for (uword i = 0; i < stack_height; i++) {
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
