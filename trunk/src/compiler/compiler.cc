#include "compiler/compiler.h"

#include "heap/ref-inl.h"
#include "runtime/runtime-inl.h"

namespace neutrino {

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
  ref<Code> code = factory().new_code(0);
  ref<Tuple> literals = factory().new_tuple(0);
  return factory().new_lambda(0, code, literals);
}

} // neutrino
