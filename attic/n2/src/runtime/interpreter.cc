#include "backends/bytecode.h"
#include "heap/ref-inl.h"
#include "main/options.h"
#include "runtime/builtins-inl.h"
#include "runtime/interpreter-inl.pp.h"
#include "runtime/runtime-inl.h"
#include "utils/array-inl.h"
#include "utils/checks.h"
#include "utils/log.h"
#include "utils/smart-ptrs-inl.h"
#include "utils/string-inl.pp.h"
#include "values/method-inl.h"

namespace neutrino {


// -------------
// --- L o g ---
// -------------

class ExecutionLog {
public:
  static inline void instruction(uword pc, array<uint16_t> code,
      array<Value*> pool, StackState &frame);
};


// -----------------------------
// --- I n t e r p r e t e r ---
// -----------------------------

Signal *Interpreter::prepare_call(ref<Task> task, ref<Lambda> lambda,
    uword argc) {
  @assert 0 == argc;
  lambda.ensure_compiled(runtime(), NULL);
  Stack *stack = task->stack();
  ASSERT(stack->status().is_empty);
  @assert stack->status().state == Stack::Status::ssParked;
  stack->status().is_empty = false;
  uword prev_pc = stack->pc();
  IF_DEBUG(stack->status().state = Stack::Status::ssRunning);
  StackState frame(stack->bound(stack->fp()), stack->bound(stack->sp()));
  frame.push(runtime().roots().vhoid()); // push the void receiver
  frame.push_activation(prev_pc, *lambda);
  frame.park(stack, 0);
  return Success::make();
}

void StackState::park(Stack *stack, uword pc) {
  @assert stack->status().state == Stack::Status::ssRunning;
  IF_DEBUG(stack->status().state = Stack::Status::ssParked);
  stack->set_fp(fp().value());
  stack->set_sp(sp().value());
  stack->set_pc(pc);
}

/**
 * Returns the class object for the given value.
 */
Layout *Interpreter::get_layout(Immediate *value) {
  if (is<Smi>(value)) return runtime().roots().smi_layout();
  else return cast<Object>(value)->layout();
}


Data *Interpreter::lookup_method(Immediate *recv, Layout *layout,
    Selector *selector) {
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
  LOG().error("%", elms(buf.raw_string()));
}


static void unresolved_global(Value *name) {
  string_buffer buf;
  buf.append("Unresolved global: ");
  name->write_on(buf, Data::UNQUOTED);
  LOG().error("%", elms(buf.raw_string()));
}


static Signal *grow_stack(Task *task, Heap &heap) {
  Stack *old_stack = task->stack();
  uword new_size = grow_value(old_stack->height());
  Data *value = heap.new_stack(new_size).data();
  if (is<Signal>(value)) return cast<Signal>(value);
  Stack *new_stack = cast<Stack>(value);
  old_stack->uncook_stack();
  // Copy the full contents of the old stack into the new
  ::memcpy(ValuePointer::address_of(new_stack),
      ValuePointer::address_of(old_stack),
      old_stack->size_in_memory());
  // Be sure to restore the correct height after it was overwritten
  // with the old one by memcpy
  new_stack->set_height(new_size);
  // Recook both stacks.  The old stack should be garbage but just in
  // case we recook it anyway.
  old_stack->recook_stack();
  new_stack->recook_stack();
  task->set_stack(new_stack);
  return Success::make();
}


class InterpreterState {
public:
  InterpreterState(Task *task) : task_(task) { }
  Task *task() { return task_; }
  Stack *stack() { return task()->stack(); }
  void attach(Task *task) { task_ = task; }
private:
  friend class CaptureInterpreterState;
  Task *task_;
};


/**
 * A utility class used to secure the members of an InterpreterState
 * during gc.
 */
class CaptureInterpreterState {
public:
  CaptureInterpreterState(RefManager &refs, InterpreterState &state)
    : protect(refs) {
    task_ = protect(state.task_);
  }
  void restore(InterpreterState &state) {
    state.task_ = *task_;
  }
private:
  ref_block<> protect;
  ref<Task> task_;
};


Value *Interpreter::call(ref<Lambda> lambda, ref<Task> initial_task) {
  prepare_call(initial_task, lambda, 0);
  InterpreterState state(*initial_task);
  StackState initial_frame(state.stack()->bound(state.stack()->fp()),
      state.stack()->bound(state.stack()->sp()));
  while (true) {
    @assert state.stack()->status().state == Stack::Status::ssParked;
    Data *value = interpret(state);
    @assert state.stack()->status().state == Stack::Status::ssParked;
    if (is<Signal>(value)) {
      if (Options::trace_signals)
        LOG().info("Signal: %", elms(value));
      if (is<StackOverflow>(value)) {
        value = grow_stack(state.task(), runtime().heap());
        // If grow_stack is not successful then we have to fall through
        // to get a gc.  Otherwise we can just continue.
        if (is<Success>(value))
          continue;
      }
      if (is<AllocationFailed>(value)) {
        CaptureInterpreterState capture(runtime().refs(), state);
        runtime().heap().memory().collect_garbage(runtime());
        capture.restore(state);
      } else {
        LOG().error("Unexpected signal: %", elms(value));
      }
    } else {
      state.stack()->status().is_empty = true;
      return cast<Value>(value);
    }
  }
}


#define RETURN(val) do { result = val; goto suspend; } while(false)

#define UPDATE_STACK_LIMIT() do {                                    \
    stack_limit = stack->bottom() + (stack->height() - 4 * StackState::kSize); \
  } while (false)

#define PREPARE_EXECUTION(value) do {                                \
    lambda = cast<Lambda>(value);                                    \
    code = cast<Code>(lambda->code())->buffer();                     \
    constant_pool = cast<Tuple>(lambda->constant_pool())->buffer();  \
  } while (false)


#define CHECK_STACK_HEIGHT(__lambda__) do {                          \
    if (frame.fp().value() + (__lambda__)->max_stack_height() >= stack_limit) \
      RETURN(StackOverflow::make(stack->height()));                  \
  } while (false)


Data *Interpreter::interpret(InterpreterState &state) {
  Stack *stack = state.stack();
  word *stack_limit;
  UPDATE_STACK_LIMIT();
  StackState frame(stack->bound(stack->fp()), stack->bound(stack->sp()));
  uword pc = frame.unpark(stack);
  Lambda *lambda;
  array<uint16_t> code;
  array<Value*> constant_pool;
  PREPARE_EXECUTION(frame.lambda());
  Data *result = Nothing::make();
  while (true) {
    IF_DEBUG(ExecutionLog::instruction(pc, code, constant_pool, frame));
    switch (code[pc]) {
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
        unresolved_global(name);
        UNREACHABLE();
      } else {
        frame.push(cast<Value>(value));
      }
      pc += OpcodeInfo<ocGlobal>::kSize;
      break;
    }
    case ocLoadLocal: {
      uint16_t index = code[pc + 1];
      Value *value = frame.local(index);
      frame.push(value);
      pc += OpcodeInfo<ocLoadLocal>::kSize;
      break;
    }
    case ocStoreLocal: {
      uint16_t index = code[pc + 1];
      frame.local(index) = frame[0];
      pc += OpcodeInfo<ocStoreLocal>::kSize;
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
      uint16_t argc = code[pc + 2];
      Selector *selector = cast<Selector>(constant_pool[selector_index]);
      Tuple *keymap = cast<Tuple>(constant_pool[code[pc + 3]]);
      Value *recv = frame[argc];
      Immediate *immed = deref(recv);
      if (is<Channel>(immed)) {
        Channel *channel = cast<Channel>(immed);
        MessageArguments args(runtime(), argc, frame);
        Data *value = channel->send(selector, args);
        if (is<Signal>(value)) {
          if (is<AllocationFailed>(value)) {
            RETURN(value);
          } else {
            LOG().error("Problem calling channel: %", elms(value));
          }
        } else {
          frame.push(cast<Value>(value));
          pc += OpcodeInfo<ocInvoke>::kSize;
        }
        break;
      } else {
        Layout *layout = get_layout(immed);
        Data *lookup_result = lookup_method(immed, layout, selector);
        if (is<Nothing>(lookup_result)) {
          scoped_string selector_str(selector->to_string());
          scoped_string recv_str(recv->to_short_string());
          LOG().error("Lookup failure: %::%",
              elms(*recv_str, *selector_str));
        }
        Method *method = cast<Method>(lookup_result);
        Lambda *target = method->lambda();
        target->ensure_compiled(runtime(), method);
        CHECK_STACK_HEIGHT(target);
        frame.push_activation(pc + OpcodeInfo<ocInvoke>::kSize, target);
        PREPARE_EXECUTION(target);
        if (!keymap->is_empty())
          frame.push(keymap);
        pc = 0;
        break;
      }
    }
    case ocInvokeSuper: {
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
        LOG().error("Lookup failure: %::%",
            elms(*recv_str, *selector_str));
      }
      Method *method = cast<Method>(lookup_result);
      Lambda *target = method->lambda();
      target->ensure_compiled(runtime(), method);
      CHECK_STACK_HEIGHT(target);
      frame.push_activation(pc + OpcodeInfo<ocInvokeSuper>::kSize, target);
      PREPARE_EXECUTION(target);
      if (!keymap->is_empty())
        frame.push(keymap);
      pc = 0;
      break;
    }
    case ocRaise: {
      uint16_t name_index = code[pc + 1];
      Value *name = constant_pool[name_index];
      uint16_t argc = code[pc + 2];
      Marker marker(state.stack()->top_marker());
      while (!marker.is_bottom()) {
        Tuple *handlers = cast<Tuple>(marker.data());
        for (uword i = 0; i < handlers->length(); i += 2) {
          String *handler = cast<String>(handlers->get(i));
          if (name->equals(handler)) {
            Lambda *target = cast<Lambda>(handlers->get(i + 1));
            target->ensure_compiled(runtime(), NULL);
            CHECK_STACK_HEIGHT(target);
            frame.push_activation(pc + OpcodeInfo<ocRaise>::kSize, target);
            PREPARE_EXECUTION(target);
            pc = 0;
            goto end;
          }
        }
        marker.unwind();
      }
      {
        frame.push_activation(0, lambda);
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
      Lambda *target = cast<Lambda>(value);
      target->ensure_compiled(runtime(), NULL);
      CHECK_STACK_HEIGHT(target);
      frame.push_activation(pc + OpcodeInfo<ocCall>::kSize, target);
      PREPARE_EXECUTION(target);
      pc = 0;
      break;
    }
    case ocNew: {
      uint16_t layout_template_index = code[pc + 1];
      InstanceLayout *layout_template = cast<InstanceLayout>(constant_pool[layout_template_index]);
      uword field_count = layout_template->instance_field_count();
      Protocol *proto = cast<Protocol>(frame[field_count]);
      Data *layout_val = layout_template->clone(runtime().heap()).data();
      if (is<AllocationFailed>(layout_val)) RETURN(layout_val);
      InstanceLayout *layout = cast<InstanceLayout>(layout_val);
      layout->set_protocol(proto);
      Data *val = runtime().heap().new_instance(layout).data();
      if (is<Signal>(val)) RETURN(val);
      Instance *instance = cast<Instance>(val);
      for (word i = field_count - 1; i >= 0; i--)
        instance->set_field(i, frame.pop());
      frame.pop(); // Pop the protocol
      frame.push(instance);
      pc += OpcodeInfo<ocNew>::kSize;
      break;
    }
    case ocLoadField: {
      uint16_t index = code[pc + 1];
      uint16_t argc = code[pc + 2];
      maybe<Instance> self = to<Instance>(frame.self(argc));
      Value *value = self.value()->get_field(index);
      frame.push(value);
      pc += OpcodeInfo<ocLoadField>::kSize;
      break;
    }
    case ocStoreField: {
      uint16_t index = code[pc + 1];
      uint16_t argc = code[pc + 2];
      Value *value = frame[0];
      maybe<Instance> self = to<Instance>(frame.self(argc));
      self.value()->set_field(index, value);
      pc += OpcodeInfo<ocStoreField>::kSize;
      break;
    }
    case ocMark: {
      uint16_t data_index = code[pc + 1];
      Value *data = constant_pool[data_index];
      Marker mark = frame.push_marker();
      mark.set_data(data);
      word *current_marker = state.stack()->top_marker();
      mark.set_prev_mp(current_marker);
      state.stack()->set_top_marker(mark.mp());
      pc += OpcodeInfo<ocMark>::kSize;
      break;
    }
    case ocUnmark: {
      Value *value = frame.pop();
      Marker mark = frame.pop_marker();
      word *marker_value = mark.prev_mp();
      ASSERT(ValuePointer::has_smi_tag(marker_value));
      state.stack()->set_top_marker(marker_value);
      frame.push(value);
      pc += OpcodeInfo<ocUnmark>::kSize;
      break;
    }
    case ocForward: {
      Forwarder::Type type = static_cast<Forwarder::Type>(code[pc + 1]);
      Value *value = frame.pop();
      Data *forw = runtime().heap().new_forwarder(type, value).data();
      if (is<AllocationFailed>(forw)) RETURN(forw);
      frame.push(cast<Value>(forw));
      pc += OpcodeInfo<ocForward>::kSize;
      break;
    }
    case ocBindForwarder: {
      Value *value = frame.pop();
      Forwarder *forw = cast<Forwarder>(frame.pop());
      forw->descriptor()->set_target(value);
      forw->descriptor()->set_type(Forwarder::fwClosed);
      frame.push(value);
      pc += OpcodeInfo<ocBindForwarder>::kSize;
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
          RETURN(value);
        } else {
          LOG().error("Problem executing builtin %: %", elms(index, value));
        }
      } else {
        frame.push(cast<Value>(value));
        pc += OpcodeInfo<ocBuiltin>::kSize;
      }
      break;
    }
    case ocAttach: {
      Task *task = cast<Task>(frame.pop());
      @assert task->stack()->status().state == Stack::Status::ssParked;
      @assert is<Null>(task->caller());
      task->set_caller(state.task());
      frame.park(state.stack(), pc + OpcodeInfo<ocAttach>::kSize);
      state.attach(task);
      pc = frame.unpark(task->stack());
      UPDATE_STACK_LIMIT();
      PREPARE_EXECUTION(frame.lambda());
      break;
    }
    case ocYield: {
      Value *value = frame[0];
      // Grab the caller of the current task
      Task *caller = cast<Task>(state.task()->caller());
      @assert caller->stack()->status().state == Stack::Status::ssParked;
      // Park the current task
      state.task()->set_caller(runtime().roots().nuhll());
      frame.park(state.stack(), pc + OpcodeInfo<ocYield>::kSize);
      // Attach the caller
      state.attach(caller);
      pc = frame.unpark(caller->stack());
      UPDATE_STACK_LIMIT();
      // Push the yielded value on the caller's stack
      frame.push(value);
      // Go!
      PREPARE_EXECUTION(frame.lambda());
      break;
    }
    case ocStackBottom: {
      Value *value = frame.pop();
      RETURN(value);
    }
    case ocReturn: {
      Value *value = frame.pop();
      pc = frame.prev_pc();
      frame.unwind(state.stack());
      PREPARE_EXECUTION(frame.lambda());
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
      Data *val = runtime().heap().new_tuple(length).data();
      if (is<Signal>(val)) RETURN(val);
      Tuple *result = cast<Tuple>(val);
      for (word i = length - 1; i >= 0; i--)
        result->set(i, frame.pop());
      frame.push(result);
      pc += OpcodeInfo<ocTuple>::kSize;
      break;
    }
    case ocCheckHeight: {
      uint16_t expected = code[pc + 1];
      @check frame.locals() == expected;
      pc += OpcodeInfo<ocCheckHeight>::kSize;
      break;
    }
    case ocConcat: {
      uword terms = code[pc + 1];
      uword length = 0;
      for (uword i = 0; i < terms; i++)
        length += cast<String>(frame[i])->length();
      Data *val = runtime().heap().new_string(length).data();
      if (is<Signal>(val)) RETURN(val);
      String *result = cast<String>(val);
      uword cursor = 0;
      for (word i = terms - 1; i >= 0; i--) {
        String *term = cast<String>(frame[i]);
        for (uword j = 0; j < term->length(); j++)
          result->set(cursor + j, term->get(j));
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
      uword outer_count = code[pc + 2];
      Data *outers_val = runtime().heap().new_tuple(outer_count).data();
      if (is<Signal>(outers_val)) RETURN(outers_val);
      Tuple *outers = cast<Tuple>(outers_val);
      for (uword i = 0; i < outer_count; i++)
        outers->set(outer_count - i - 1, frame.pop());
      Data *clone_val = lambda->clone(runtime().heap()).data();
      if (is<Signal>(clone_val)) RETURN(clone_val);
      Lambda *clone = cast<Lambda>(clone_val);
      clone->set_outers(outers);
      frame.push(clone);
      pc += OpcodeInfo<ocClosure>::kSize;
      break;
    }
    case ocTask: {
      ref_block<> protect(runtime().refs());
      Lambda *lambda = cast<Lambda>(frame.pop());
      Data *task_val = runtime().heap().new_task(runtime().architecture()).data();
      if (is<Signal>(task_val)) RETURN(task_val);
      Task *task = cast<Task>(task_val);
      prepare_call(protect(task), protect(lambda), 0);
      frame.push(task);
      pc += OpcodeInfo<ocTask>::kSize;
      break;
    }
    case ocOuter: {
      uword index = code[pc + 1];
      Value *value = lambda->outers()->get(index);
      frame.push(value);
      pc += OpcodeInfo<ocOuter>::kSize;
      break;
    }
    case ocNewCell: {
      Value *value = frame.pop();
      Data *val = runtime().heap().new_cell(value).data();
      if (is<AllocationFailed>(val)) RETURN(val);
      frame.push(cast<Cell>(val));
      pc += OpcodeInfo<ocNewCell>::kSize;
      break;
    }
    case ocLoadCell: {
      Cell *cell = cast<Cell>(frame.pop());
      frame.push(cell->value());
      pc += OpcodeInfo<ocLoadCell>::kSize;
      break;
    }
    case ocStoreCell: {
      Cell *cell = cast<Cell>(frame.pop());
      cell->set_value(frame[0]);
      pc += OpcodeInfo<ocStoreCell>::kSize;
      break;
    }
    default:
      UNHANDLED(Opcode, code[pc]);
      RETURN(Nothing::make());
    }
  }
suspend:
  frame.park(state.stack(), pc);
  return result;
}


#undef RETURN
#undef PREPARE_EXECUTION
#undef CHECK_STACK_HEIGHT


#ifdef DEBUG
void Stack::validate_stack() {
  if (status().is_empty) return;
  ASSERT(status().is_cooked);
  StackState frame(bound(fp()));
  while (true) {
    GC_SAFE_CHECK_IS_C(cnValidation, Lambda, frame.lambda());
    if (frame.is_bottom()) break;
    uword local_count = frame.locals();
    for (uword i = 0; i < local_count; i++)
      GC_SAFE_CHECK_IS_C(cnValidation, Value, frame[i]);
    frame.unwind(this);
  }
}
#endif

void Stack::recook_stack() {
  if (status().is_empty) return;
  ASSERT(!status().is_cooked);
  @assert status().state == Stack::Status::ssParked;
  array<word> buf = buffer();
  set_fp(buf.from_offset(reinterpret_cast<uword>(fp())));
  set_sp(buf.from_offset(reinterpret_cast<uword>(sp())));
  StackState frame(bound(fp()));
  while (!frame.is_bottom()) {
    frame.prev_fp() = buf.from_offset(reinterpret_cast<uword>(frame.prev_fp()));
    frame.unwind(this);
  }
  IF_DEBUG(status().is_cooked = true);
}

void Stack::uncook_stack() {
  if (status().is_empty) return;
  ASSERT(status().is_cooked);
  @assert status().state == Stack::Status::ssParked;
  StackState frame(bound(fp()), bound(sp()));
  array<word> buf = buffer();
  while (!frame.is_bottom()) {
    IF_DEBUG(word *prev_fp = frame.prev_fp());
    frame.prev_fp() = reinterpret_cast<word*>(buf.offset_of(frame.prev_fp()));
    frame.unwind(buf, height());
    ASSERT(prev_fp == frame.fp().value());
  }
  set_fp(reinterpret_cast<word*>(buf.offset_of(fp())));
  set_sp(reinterpret_cast<word*>(buf.offset_of(sp())));
  IF_DEBUG(status().is_cooked = false);
}

void Stack::for_each_stack_field(FieldVisitor &visitor) {
  if (status().is_empty) return;
  ASSERT(!status().is_cooked);
  @assert status().state == Stack::Status::ssParked;
  array<word> buf = buffer();
  StackState frame(bound(buf.from_offset(reinterpret_cast<uword>(fp()))),
      bound(buf.from_offset(reinterpret_cast<uword>(sp()))));
  while (true) {
    visitor.visit_field(pointer_cast<Value*>(&frame.lambda()));
    if (frame.is_bottom()) break;
    uword stack_height = frame.locals();
    for (uword i = 0; i < stack_height; i++) {
      // Visit the i'th local variable
      visitor.visit_field(&frame[i]);
    }
    frame.unwind(buf, height());
  }
}


MAKE_ENUM_INFO_HEADER(Opcode)
#define MAKE_ENTRY(n, Name, argc, argf) MAKE_ENUM_INFO_ENTRY(oc##Name)
eOpcodes(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()


void OpcodeData::load(uint16_t value) {
  switch (value) {
#define MAKE_CASE(n, Name, argc, argf)                               \
    case oc##Name:                                                   \
      name_ = #Name;                                                 \
      format_ = argf;                                                \
      length_ = argc + 1;                                            \
      is_resolved_ = true;                                           \
      break;
eOpcodes(MAKE_CASE)
#undef MAKE_CASE
  }
}


void ExecutionLog::instruction(uword pc, array<uint16_t> code,
    array<Value*> pool, StackState &frame) {
  if (Options::trace_interpreter) {
    string_buffer buf;
    BytecodeBackend::disassemble_next_instruction(&pc, code, pool, buf);
    buf.raw_string().print();
  }
}

} // namespace neutrino