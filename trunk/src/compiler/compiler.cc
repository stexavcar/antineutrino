#include "compiler/compiler.h"
#include "heap/ref-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"

namespace neutrino {

class CompileSession {
public:
  CompileSession(Runtime &runtime);
  ref<Lambda> compile(ref<SyntaxTree> tree);
private:
  Factory &factory() { return factory_; }
  Factory &factory_;
};

class heap_list {
public:
  heap_list(Factory &factory) : factory_(factory) { }
  void initialize();
  ~heap_list() { data().dispose(); }
private:
  Factory &factory() { return factory_; }
  permanent<Tuple> data() { return data_; }
  Factory &factory_;
  permanent<Tuple> data_;
};

void heap_list::initialize() {
  ref<Tuple> tuple = factory().new_tuple(16);
  data_ = new_permanent(*tuple);
}

class Assembler {
public:
  Assembler(Factory &factory) : literals_(factory) { }
  void initialize() { literals().initialize(); }
private:
  heap_list &literals() { return literals_; }
  list_buffer<uint32_t> code_;
  heap_list literals_;
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
  ref<Code> code = factory().new_code(0);
  ref<Tuple> literals = factory().new_tuple(0);
  return factory().new_lambda(0, code, literals);
}

} // neutrino
