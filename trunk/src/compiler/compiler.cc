#include "compiler/ast-inl.h"
#include "compiler/compiler.h"
#include "compiler/compile-utils-inl.h"
#include "heap/ref-inl.h"
#include "runtime/interpreter-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"

namespace neutrino {

// -------------------------
// --- A s s e m b l e r ---
// -------------------------

#define __ this->

class Assembler : public Visitor {
public:
  Assembler(Factory &factory) : factory_(factory), pool_(factory) { }
  void initialize() { pool().initialize(); }
  void codegen(ref<SyntaxTree> that) { that.accept(*this); }
  
  ref<Code> flush_code();
  ref<Tuple> flush_constant_pool();
  
  uint16_t constant_pool_index(ref<Value> value);
  
  void push(uint16_t index);
  void pop(uint16_t height = 1);
  void rethurn();
  
  virtual void visit_literal_expression(ref<LiteralExpression> that);
  virtual void visit_return_expression(ref<ReturnExpression> that);
  virtual void visit_sequence_expression(ref<SequenceExpression> that);
private:
  Factory &factory() { return factory_; }
  heap_list &pool() { return pool_; }
  list_buffer<uint16_t> &code() { return code_; }
  Factory &factory_;
  list_buffer<uint16_t> code_;
  heap_list pool_;
};

ref<Code> Assembler::flush_code() {
  ref<Code> result = factory().new_code(code().length());
  for (uint32_t i = 0; i < result.length(); i++)
    result.at(i) = code()[i];
  return result;
}

ref<Tuple> Assembler::flush_constant_pool() {
  ref<Tuple> result = factory().new_tuple(pool().length());
  for (uint32_t i = 0; i < result.length(); i++)
    result->set(i, pool().get(i));
  return result;
}

uint16_t Assembler::constant_pool_index(ref<Value> value) {
  heap_list &pool = this->pool();
  for (uint16_t i = 0; i < pool.length(); i++) {
    RefScope scope;
    ref<Value> entry = pool[i];
    if (entry->equals(*value)) return i;
  }
  uint16_t result = pool.length();
  pool.append(value);
  return result;
}

void Assembler::push(uint16_t index) {
  STATIC_CHECK(OpcodeInfo<OC_PUSH>::kArgc == 1);
  code().append(OC_PUSH);
  code().append(index);
}

void Assembler::pop(uint16_t height) {
  STATIC_CHECK(OpcodeInfo<OC_POP>::kArgc == 1);
  code().append(OC_POP);
  code().append(height);
}

void Assembler::rethurn() {
  STATIC_CHECK(OpcodeInfo<OC_RETURN>::kArgc == 0);
  code().append(OC_RETURN);
}


// -------------------------------------
// --- C o d e   G e n e r a t i o n ---
// -------------------------------------

void Assembler::visit_return_expression(ref<ReturnExpression> that) {
  __ codegen(that.value());
  __ rethurn();
  
}

void Assembler::visit_literal_expression(ref<LiteralExpression> that) {
  uint16_t index = constant_pool_index(that.value());
  __ push(index);
}

void Assembler::visit_sequence_expression(ref<SequenceExpression> that) {
  RefScope scope;
  ref<Tuple> expressions = that.expressions();
  ASSERT(expressions.length() > 1);
  bool is_first = true;
  for (uint32_t i = 0; i < expressions.length(); i++) {
    if (is_first) {
      is_first = false;
    } else {
      __ pop();
    }
    __ codegen(cast<SyntaxTree>(expressions.get(i)));
  }
}


// -----------------------
// --- C o m p i l e r ---
// -----------------------

class CompileSession {
public:
  CompileSession(Runtime &runtime);
  ref<Lambda> compile(ref<SyntaxTree> tree);
private:
  Factory &factory() { return factory_; }
  Factory &factory_;
};

CompileSession::CompileSession(Runtime &runtime)
    : factory_(runtime.factory()) { }

ref<Lambda> Compiler::compile(ref<SyntaxTree> tree) {
  CompileSession session(Runtime::current());
  return session.compile(tree);
}

ref<Lambda> CompileSession::compile(ref<SyntaxTree> tree) {
  Assembler assembler(factory());
  assembler.initialize();
  tree.accept(assembler);
  ref<Code> code = assembler.flush_code();
  ref<Tuple> constant_pool = assembler.flush_constant_pool();
  return factory().new_lambda(0, code, constant_pool);
}

} // neutrino
