#ifndef _METHOD_INL
#define _METHOD_INL

#include "values/values-inl.pp.h"

namespace neutrino {

MethodLookup::MethodLookup(uword min_score)
  : score_(kNoMatch)
  , min_score_(min_score)
  , method_(Nothing::make())
  , is_ambiguous_(false) { }


uword MethodLookup::get_distance(Layout *layout, Protocol *target) {
  Value *current = layout->protocol();
  word distance = 0;
  while (is<Protocol>(current)) {
    if (current == target) return distance;
    distance++;
    current = cast<Protocol>(current)->super();
  }
  return MethodLookup::kNoMatch;
}


uword MethodLookup::get_method_score(Signature *signature, Layout *receiver) {
  uword score = 0;
  for (uword i = 0; i < signature->parameters()->length(); i++) {
    Value *param = signature->parameters()->get(i);
    if (is<Protocol>(param)) {
      uword distance = get_distance(receiver, cast<Protocol>(param));
      if (distance != kNoMatch) score += distance;
      else return kNoMatch;
    }
  }
  return score;
}


} // neutrino

#endif // _METHOD_INL
