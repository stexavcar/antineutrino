#include "heap/ref-inl.h"
#include "runtime/builtins-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/checks.h"
#include "utils/scoped-ptrs-inl.h"
#include "utils/vector-inl.h"
#include "values/method-inl.h"

namespace neutrino {

MAKE_ENUM_INFO_HEADER(Opcode)
#define MAKE_ENTRY(n, Name, argc) MAKE_ENUM_INFO_ENTRY(oc##Name)
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
  ref_scope scope(runtime().refs());
  ref<Task> task = runtime().refs().new_ref(initial_task);
  ref<Lambda> lambda = runtime().refs().new_ref(initial_lambda);
  Frame frame = prepare_call(task, lambda, 0);
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
        runtime().heap().memory().collect_garbage(runtime());
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

Frame Interpreter::prepare_call(ref<Task> task, ref<Lambda> lambda, uword argc) {
  lambda.ensure_compiled(runtime(), NULL);
  ASSERT(task->stack()->status().is_empty);
  task->stack()->status().is_empty = false;
  Frame frame(task->stack()->bottom() + Frame::accessible_below_fp(argc));
  frame.self(argc) = runtime().roots().vhoid();
  frame.lambda() = *lambda;
  frame.prev_fp() = 0;
  ASSERT(frame.sp() >= frame.fp() + Frame::kSize);
  return frame;
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


Data *Interpreter::lookup_method(Layout *layout, Selector *selector) {
  MethodLookup lookup(0);
  lookup.lookup_method(layout, selector);
  ASSERT(!lookup.is_ambiguous());
  return lookup.method();
}


Data *Interpreter::lookup_super_method(Layout *layout, Selector *selector,
    Signature *current) {
  uword prev_score = MethodLookup::get_method_score(current, layout);
  ASSERT(prev_score != MethodLookup::kNoMatch);
  MethodLookup lookup(prev_score + 1);
  lookup.lookup_method(layout, selector);
  ASSERT(!lookup.is_ambiguous());
  return lookup.method();
}


static void unhandled_condition(Value *name, BuiltinArguments &args) {
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
    case ocPush: {
      uint16_t index = code[pc + 1];
      Value *value = constant_pool[index];
      frame.push(value);
      pc += OpcodeInfo<ocPush>::kSize;
      break;
    }
    case ocSlap: {
      uint16_t height = code[pc + 1];
      frame[height] = frame[0];
      frame.pop(height);
      pc += OpcodeInfo<ocSlap>::kSize;
      break;
    }
    case ocSwap: {
      Value *temp = frame[0];
      frame[0] = frame[1];
      frame[1] = temp;
      pc += OpcodeInfo<ocSwap>::kSize;
      break;
    }
    case ocGlobal: {
      uint16_t index = code[pc + 1];
      Value *name = constant_pool[index];
      Data *value = runtime().toplevel()->get(name);
      if (is<Nothing>(value)) {
        frame.push(runtime().roots().vhoid());
      } else {
        frame.push(cast<Value>(value));
      }
      pc += OpcodeInfo<ocGlobal>::kSize;
      break;
    }
    case ocLdLocal: {
      uint16_t index = code[pc + 1];
      Value *value = frame.local(index);
      frame.push(value);
      pc += OpcodeInfo<ocLdLocal>::kSize;
      break;
    }
    case ocStLocal: {
      uint16_t index = code[pc + 1];
      frame.local(index) = frame[0];
      pc += OpcodeInfo<ocStLocal>::kSize;
      break;
    }
    case ocArgument: {
      uint16_t index = code[pc + 1];
      Value *value = frame.argument(index);
      frame.push(value);
      pc += OpcodeInfo<ocArgument>::kSize;
      break;
    }
    case ocKeyword: {
      uint16_t index = code[pc + 1];
      Tuple *keymap = cast<Tuple>(frame.local(0));
      uword offset = cast<Smi>(keymap->get(index))->value();
      frame.push(frame.argument(offset));
      pc += OpcodeInfo<ocKeyword>::kSize;
      break;
    }
    case ocIfTrue: {
      Value *value = frame.pop();
      if (is<True>(value)) {
        uint16_t index = code[pc + 1];
        pc = index;
      } else {
        pc += OpcodeInfo<ocIfTrue>::kSize;
      }
      break;
    }
    case ocIfFalse: {
      Value *value = frame.pop();
      if (is<False>(value)) {
        uint16_t index = code[pc + 1];
        pc = index;
      } else {
        pc += OpcodeInfo<ocIfFalse>::kSize;
      }
      break;
    }
    case ocGoto: {
      uint16_t address = code[pc + 1];
      pc = address;
      break;
    }
    case ocInvoke: {
      uint16_t selector_index = code[pc + 1];
      Selector *selector = cast<Selector>(constant_pool[selector_index]);
      Tuple *keymap = cast<Tuple>(constant_pool[code[pc + 3]]);
      uint16_t argc = code[pc + 2];
      Value *recv = frame[argc];
      Layout *layout = get_layout(deref(recv));
      Data *lookup_result = lookup_method(layout, selector);
      if (is<Nothing>(lookup_result)) {
        scoped_string selector_str(selector->to_string());
        scoped_string recv_str(recv->to_short_string());
        Conditions::get().error_occurred("Lookup failure: %s::%s",
            recv_str.chars(), selector_str.chars());
      }
      Method *method = cast<Method>(lookup_result);
      frame.push_activation();
      frame.prev_pc() = pc + OpcodeInfo<ocInvoke>::kSize;
      frame.lambda() = method->lambda();
      method->lambda()->ensure_compiled(runtime(), method);
      code = cast<Code>(method->lambda()->code())->buffer();
      constant_pool = cast<Tuple>(method->lambda()->constant_pool())->buffer();
      if (!keymap->is_empty())
        frame.push(keymap);
      pc = 0;
      break;
    }
    case ocInvSup: {
      uint16_t selector_index = code[pc + 1];
      Selector *selector = cast<Selector>(constant_pool[selector_index]);
      Tuple *keymap = cast<Tuple>(constant_pool[code[pc + 3]]);
      Signature *current = cast<Signature>(constant_pool[code[pc + 4]]);
      uint16_t argc = code[pc + 2];
      Value *recv = frame[argc];
      Layout *layout = get_layout(deref(recv));
      Data *lookup_result = lookup_super_method(layout, selector, current);
      if (is<Nothing>(lookup_result)) {
        scoped_string selector_str(selector->to_string());
        scoped_string recv_str(recv->to_short_string());
        Conditions::get().error_occurred("Lookup failure: %s::%s",
            recv_str.chars(), selector_str.chars());
      }
      Method *method = cast<Method>(lookup_result);
      frame.push_activation();
      frame.prev_pc() = pc + OpcodeInfo<ocInvSup>::kSize;
      frame.lambda() = method->lambda();
      method->lambda()->ensure_compiled(runtime(), method);
      code = cast<Code>(method->lambda()->code())->buffer();
      constant_pool = cast<Tuple>(method->lambda()->constant_pool())->buffer();
      if (!keymap->is_empty())
        frame.push(keymap);
      pc = 0;
      break;
    }
    case ocRaise: {
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
            lambda->ensure_compiled(runtime(), NULL);
            frame.push_activation();
            frame.prev_pc() = pc + OpcodeInfo<ocRaise>::kSize;
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
        frame.push_activation();
        BuiltinArguments args(runtime(), argc, frame);
        unhandled_condition(name, args);
        UNREACHABLE();
      }
     end:
      break;
    }
    case ocCall: {
      uint16_t argc = code[pc + 1];
      Value *value = frame[argc + 1];
      Lambda *fun = cast<Lambda>(value);
      fun->ensure_compiled(runtime(), NULL);
      frame.push_activation();
      frame.prev_pc() = pc + OpcodeInfo<ocCall>::kSize;
      frame.lambda() = fun;
      code = cast<Code>(fun->code())->buffer();
      constant_pool = cast<Tuple>(fun->constant_pool())->buffer();
      pc = 0;
      break;
    }
    case ocAttach: {
      UNREACHABLE();
    }
    case ocNew: {
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
      pc += OpcodeInfo<ocNew>::kSize;
      break;
    }
    case ocLdField: {
      uint16_t index = code[pc + 1];
      uint16_t argc = code[pc + 2];
      Value *value = cast<Instance>(to<Instance>(frame.self(argc)))->get_field(index);
      frame.push(value);
      pc += OpcodeInfo<ocLdField>::kSize;
      break;
    }
    case ocStField: {
      uint16_t index = code[pc + 1];
      uint16_t argc = code[pc + 2];
      Value *value = frame[0];
      cast<Instance>(to<Instance>(frame.self(argc)))->set_field(index, value);
      pc += OpcodeInfo<ocStField>::kSize;
      break;
    }
    case ocMark: {
      uint16_t data_index = code[pc + 1];
      Value *data = constant_pool[data_index];
      Marker mark = frame.push_marker();
      mark.set_data(data);
      word *current_marker = stack->top_marker();
      mark.set_prev_mp(current_marker);
      stack->set_top_marker(mark.mp());
      pc += OpcodeInfo<ocMark>::kSize;
      break;
    }
    case ocUnmark: {
      Value *value = frame.pop();
      Marker mark = frame.pop_marker();
      word *marker_value = mark.prev_mp();
      ASSERT(ValuePointer::has_smi_tag(marker_value));
      stack->set_top_marker(marker_value);
      frame.push(value);
      pc += OpcodeInfo<ocUnmark>::kSize;
      break;
    }
    case ocForward: {
      Forwarder::Type type = static_cast<Forwarder::Type>(code[pc + 1]);
      Value *value = frame.pop();
      Data *forw = runtime().heap().new_forwarder(type, value);
      if (is<AllocationFailed>(forw)) return forw;
      frame.push(cast<Value>(forw));
      pc += OpcodeInfo<ocForward>::kSize;
      break;
    }
    case ocBindFor: {
      Value *value = frame.pop();
      Forwarder *forw = cast<Forwarder>(frame.pop());
      forw->descriptor()->set_target(value);
      forw->descriptor()->set_type(Forwarder::fwClosed);
      frame.push(value);
      pc += OpcodeInfo<ocBindFor>::kSize;
      break;
    }
    case ocBuiltin: {
      uint16_t argc = code[pc + 1];
      uint16_t index = code[pc + 2];
      builtin *builtin = Builtins::get(index);
      BuiltinArguments args(runtime(), argc, frame);
      Data *value = builtin(args);
      if (is<Signal>(value)) {
        if (is<AllocationFailed>(value)) {
          return value;
        } else {
          Conditions::get().error_occurred("Problem executing builtin %i", index);
        }
      } else {
        frame.push(cast<Value>(value));
        pc += OpcodeInfo<ocBuiltin>::kSize;
      }
      break;
    }
    case ocYield: case ocReturn: {
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
    case ocVoid: {
      frame.push(runtime().roots().vhoid());
      pc += OpcodeInfo<ocVoid>::kSize;
      break;
    }
    case ocNull: {
      frame.push(runtime().roots().nuhll());
      pc += OpcodeInfo<ocNull>::kSize;
      break;
    }
    case ocTrue: {
      frame.push(runtime().roots().thrue());
      pc += OpcodeInfo<ocTrue>::kSize;
      break;
    }
    case ocFalse: {
      frame.push(runtime().roots().fahlse());
      pc += OpcodeInfo<ocFalse>::kSize;
      break;
    }
    case ocPop: {
      uint16_t height = code[pc + 1];
      frame.pop(height);
      pc += OpcodeInfo<ocPop>::kSize;
      break;
    }
    case ocTuple: {
      uint16_t length = code[pc + 1];
      Data *val = runtime().heap().new_tuple(length);
      if (is<Signal>(val)) return val;
      Tuple *result = cast<Tuple>(val);
      for (word i = length - 1; i >= 0; i--)
        result->set(i, frame.pop());
      frame.push(result);
      pc += OpcodeInfo<ocTuple>::kSize;
      break;
    }
    case ocChkHgt: {
      uint16_t expected = code[pc + 1];
      CHECK_EQ(expected, frame.locals());
      pc += OpcodeInfo<ocChkHgt>::kSize;
      break;
    }
    case ocConcat: {
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
      pc += OpcodeInfo<ocConcat>::kSize;
      break;
    }
    case ocClosure: {
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
      pc += OpcodeInfo<ocClosure>::kSize;
      break;
    }
    case ocTask: {
      Lambda *lambda = cast<Lambda>(frame.pop());
      Data *task_val = runtime().heap().new_task();
      if (is<Signal>(task_val)) return task_val;
      Task *task = cast<Task>(task_val);
      ref_scope scope(runtime().refs());
      prepare_call(runtime().refs().new_ref(task), runtime().refs().new_ref(lambda), 0);
      frame.push(task);
      pc += OpcodeInfo<ocTask>::kSize;
      break;
    }
    case ocOuter: {
      uword index = code[pc + 1];
      Value *value = frame.lambda()->outers()->get(index);
      frame.push(value);
      pc += OpcodeInfo<ocOuter>::kSize;
      break;
    }
    case ocQuote: {
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
      pc += OpcodeInfo<ocQuote>::kSize;
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
    GC_SAFE_CHECK_IS_C(cnValidation, Lambda, frame.lambda());
    if (frame.is_bottom()) break;
    uword local_count = frame.locals();
    for (uword i = 0; i < local_count; i++)
      GC_SAFE_CHECK_IS_C(cnValidation, Value, frame[i]);
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
