#include "values/method-inl.h"

namespace neutrino {

bool Selector::selector_equals(Selector *that) {
  return name()->equals(that->name())
      && argc() == that->argc()
      && keywords()->equals(that->keywords())
      && is_accessor()->equals(that->is_accessor());
}


void MethodLookup::score_method(Method *method, Layout *receiver) {
  uword score = get_method_score(method->signature(), receiver);
  if (score == kNoMatch || score < min_score_) return;
  if (score < score_) {
    score_ = score;
    method_ = method;
    is_ambiguous_ = false;
  } else if (score == score_) {
    is_ambiguous_ = true;
  }
}


void MethodLookup::lookup_in_dictionary(Tuple *methods, Selector *selector,
    Layout *reciever) {
  for (uword i = 0; i < methods->length(); i++) {
    Method *method = cast<Method>(methods->get(i));
    if (method->selector()->equals(selector))
      score_method(method, reciever);
  }
}


/**
 * Returns the method with the specified name in the given class.
 */
void MethodLookup::lookup_method(Layout *layout, Selector *selector) {
  // Look up any layout-local methods
  lookup_in_dictionary(layout->methods(), selector, layout);
  // Look up methods in protocols
  Value *current = layout->protocol();
  while (is<Protocol>(current)) {
    Protocol *protocol = cast<Protocol>(current);
    lookup_in_dictionary(protocol->methods(), selector, layout);
    current = protocol->super();
  }
}

} // neutrino
