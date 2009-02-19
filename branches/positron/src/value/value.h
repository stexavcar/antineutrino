#ifndef _VALUE_VALUE
#define _VALUE_VALUE

#include "utils/global.h"

namespace neutrino {

class Data {

};

template <class C>
static inline bool is(Data *data);

template <class C>
static inline C *cast(Data *data);

class Value : public Data {
public:
  enum Type { tDescriptor, tString, tArray, tInstance };
};

template <> struct coerce<Value::Type> { typedef word type; };

class Object : public Value {
public:
  Object(Descriptor *descriptor) : descriptor_(descriptor) { }
  inline Type type();
  Descriptor *descriptor() { return descriptor_; }
  void set_descriptor(Descriptor *v) { descriptor_ = v; }
private:
  Descriptor *descriptor_;
};

class Instance : public Object {
private:
};

class Descriptor : public Object {
public:
  Descriptor(Descriptor *descriptor, Type instance_type)
    : Object(descriptor)
    , instance_type_(instance_type) { }
  Type instance_type() { return instance_type_; }
  virtual allocation<Object> clone_object(Object *obj, Space &space) = 0;
  virtual allocation<Descriptor> clone(Space &space) = 0;
private:
  Type instance_type_;
};

class DescriptorDescriptor : public Descriptor {
public:
  DescriptorDescriptor()
    : Descriptor(NULL, tDescriptor) { }
  virtual allocation<Object> clone_object(Object *obj, Space &space);
  virtual allocation<Descriptor> clone(Space &space);
};

class InstanceDescriptor : public Descriptor {
public:
  InstanceDescriptor(Descriptor *descriptor, word field_count)
    : Descriptor(descriptor, tInstance)
    , field_count_(field_count) { }
  word field_count() { return field_count_; }
  virtual allocation<Object> clone_object(Object *obj, Space &space);
  virtual allocation<Descriptor> clone(Space &space);
private:
  word field_count_;
};

class String : public Object {

};

class Array : public Object {

};

class Signal : public Data {
public:
  enum Type { sSuccess, sInternalError, sFatalError };
  inline Type type();
  inline word payload();
};

template <> struct coerce<Signal::Type> { typedef word type; };

class Success : public Signal {
public:
  static inline Success *make();
};

class Failure : public Signal {
};

class InternalError : public Failure {
public:
  enum Type { ieUnknown, ieSystem, ieEnvironment, ieHeapExhaustion };
  static inline InternalError *make(Type type);
};

template <> struct coerce<InternalError::Type> { typedef word type; };

class FatalError : public Failure {
public:
  enum Type { feOutOfMemory, feAbort };
  static inline FatalError *make(Type type);
};

template <> struct coerce<FatalError::Type> { typedef word type; };

} // namespace neutrino

#endif // _VALUE_VALUE
