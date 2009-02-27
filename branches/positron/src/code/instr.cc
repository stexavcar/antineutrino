#include "code/instr.h"
#include "runtime/ref-inl.h"
#include "utils/string-inl.h"
#include "value/value-inl.h"

namespace neutrino {

bool Instructions::disassemble(vector<code_t> code, ref<Array> literals,
    string_stream &out) {
  DisassembleContext context(literals, out);
  code_t *pc = code.start();
  while (pc < code.end()) {
    if (pc > code.start()) out.add('\n');
    out.add("%: ", vargs(pc - code.start()));
    instr<0> &inst = *reinterpret_cast<instr<0>*>(pc);
    switch (inst.type()) {
#define MAKE_CASE(name, Name)                                        \
      case instr<0>::it##Name: {                                     \
        name &__op__ = *reinterpret_cast<name*>(pc);                  \
        __op__.disassemble(context);                                 \
        pc += __op__.size();                                         \
        break;                                                       \
      }
eInstructions(MAKE_CASE)
  default:
    return false;
#undef MAKE_CASE
    }
  }
  return true;
}

void literal::disassemble(DisassembleContext &context) {
  context.out().add("literal %", vargs(index()));
}

void global::disassemble(DisassembleContext &context) {
  Value *str = context.get_literal(name());
  context.out().add("global %{q}", vargs(str));
}

void send::disassemble(DisassembleContext &context) {
  Value *str = context.get_literal(name());
  context.out().add("send @@%.%(", vargs(receiver(), str));
  for (word i = 0; i < argc(); i++) {
    if (i > 0) context.out().add(' ');
    context.out().add("@@%", vargs(args()[i]));
  }
  context.out().add(')');
}

void if_false::disassemble(DisassembleContext &context) {
  context.out().add("if-false @@%", vargs(target()));
}

void ghoto::disassemble(DisassembleContext &context) {
  context.out().add("goto @@%", vargs(target()));
}

void if_true_false::disassemble(DisassembleContext &context) {
  context.out().add("if-true-false @@% ? @@% : @@%", vargs(condition(),
      then_part(), else_part()));
}

} // namespace neutrino
