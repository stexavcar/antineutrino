#ifndef _BACKENDS_INTERPRET
#define _BACKENDS_INTERPRET

#include "backends/backend.h"
#include "compiler/compile-utils.h"
#include "utils/checks.h"
#include "utils/globals.h"

namespace neutrino {


class BytecodeLabel {
public:
  BytecodeLabel() : is_bound_(false) , value_(kNoTarget) { }
  bool is_bound() { return is_bound_; }
  uword value() { return value_; }
  void set_value(uword addr) { ASSERT(addr != kNoTarget); value_ = addr; }
  static const uint16_t kNoTarget = TypeConsts<uint16_t>::kMax;
private:
  bool is_bound_;
  uint16_t value_;
};


class BytecodeBackend : public AbstractBackend {
public:
  BytecodeBackend(Runtime &runtime);
  void push(ref<Value> value);
  void pop(uint16_t height = 1);
  void slap(uint16_t height);
  void swap();
  void rethurn();
  void attach();
  void invoke(ref<Selector> selector, uint16_t argc, ref<Tuple> keymap);
  void invoke_super(ref<Selector> selector, uint16_t argc, ref<Tuple> keymap, ref<Signature> current);
  void instantiate(ref<Layout> layout);
  void raise(ref<String> name, uint16_t argc);
  void call(uint16_t argc);
  void tuple(uint16_t size);
  void global(ref<Value> name);
  void argument(uint16_t index);
  void keyword(uint16_t index);
  void load_local(uint16_t height);
  void store_local(uint16_t height);
  void outer(uint16_t index);
  void closure(ref<Lambda> lambda, uint16_t outers);
  void task();
  void yield();
  void if_true(BytecodeLabel &label);
  void if_false(BytecodeLabel &label);
  void ghoto(BytecodeLabel &label);
  void bind(BytecodeLabel &label);
  void builtin(uint16_t argc, uint16_t index);
  void concat(uint16_t terms);
  void mark(ref<Value> data);
  void unmark();
  void new_forwarder(uint16_t type);
  void bind_forwarder();
  void store_cell();
  void load_cell();
  void new_cell();
  
  ref<Method> field_getter(uword index,  ref<Selector> selector,
      ref<Signature> signature, ref<Context> context);
  ref<Method> field_setter(uword index,  ref<Selector> selector,
      ref<Signature> signature, ref<Context> context);
  
  void initialize() { pool().initialize(); }
  ref<Code> flush_code();
  ref<Tuple> flush_constant_pool();

  uint16_t constant_pool_index(ref<Value> value);
  
  static void disassemble_next_instruction(uword *pc_ptr, 
      vector<uint16_t> data, vector<Value*> pool, string_buffer &buf);
  
  void adjust_stack_height(word delta);

private:
  heap_list &pool() { return pool_; }
  list_buffer<uint16_t> &code() { return code_; }
  list_buffer<uint16_t> code_;
  heap_list pool_;
};


class BytecodeCodeGeneratorConfig {
public:
  typedef BytecodeLabel Label;
  typedef BytecodeBackend Backend;
};


} // neutrino

#endif // _BACKENDS_INTERPRET
