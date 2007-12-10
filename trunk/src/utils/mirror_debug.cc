#include "utils/mirror.h"

#include "heap/values-inl.h"

namespace neutrino {

DataMirror &Data::mirror() {
  return *(new DataMirror(gc_safe_type()));
}

ClassMirror &Class::mirror() {
  return *(new ClassMirror(instance_type()));
}

ObjectMirror &Object::mirror() {
  return *(new ObjectMirror(gc_safe_type(), chlass()));
}

StackMirror &Stack::mirror() {
  return *(new StackMirror(height(), fp(), flags()));
}

} // neutrino
