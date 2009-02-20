#ifndef _VALUE_VALUE
#define _VALUE_VALUE

#include "utils/array.h"
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

/* --- O b j e c t --- */

class Object : public Value {
public:
  inline explicit Object(Descriptor *descriptor);
  inline Type type();
  inline Descriptor *descriptor();
  inline void set_descriptor(Descriptor *v);
  inline void set_forwarding_header(ForwardPointer *v);
  Data *&header() { return header_; }
private:
  Data *header_;
};

class Descriptor : public Object {
public:
  Descriptor(Descriptor *meta, Type instance_type)
    : Object(meta)
    , instance_type_(instance_type) { }
  Type instance_type() { return instance_type_; }
  virtual allocation<Object> clone_object(Object *obj, Space &space) = 0;
  virtual allocation<Descriptor> clone(Space &space) = 0;
  virtual word size_in_memory(Object *obj) = 0;
  virtual void migrate_fields(Object *obj, FieldMigrator &migrator);
private:
  Type instance_type_;
};

Object::Object(Descriptor *descriptor)
  : header_(descriptor) {
  printf("Initializing descriptor of %x: %x, header: %x\n", this, descriptor, header_);
}

template <typename O, typename S>
class DescriptorImpl : public Descriptor {
public:
  DescriptorImpl(Descriptor *meta, Type instance_type)
    : Descriptor(meta, instance_type) { }
  virtual allocation<Descriptor> clone(Space &space);
  virtual word size_in_memory(Object *obj);
};

class DescriptorDescriptor : public DescriptorImpl<Descriptor, DescriptorDescriptor> {
public:
  DescriptorDescriptor()
    : DescriptorImpl<Descriptor, DescriptorDescriptor>(NULL, tDescriptor) { }
  virtual allocation<Object> clone_object(Object *obj, Space &space);
};

/* --- I n s t a n c e --- */

class Instance : public Object {
public:
  static inline word size_in_memory(word fields);
};

class InstanceDescriptor : public DescriptorImpl<Instance, InstanceDescriptor> {
public:
  InstanceDescriptor(Descriptor *meta, word field_count)
    : DescriptorImpl<Instance, InstanceDescriptor>(meta, tInstance)
    , field_count_(field_count) { }
  word field_count() { return field_count_; }
  virtual allocation<Object> clone_object(Object *obj, Space &space);
  virtual word size_in_memory(Object *obj);
private:
  word field_count_;
};

/* --- S t r i n g --- */

class String : public Object {
public:
  String(Descriptor *descriptor, word length)
    : Object(descriptor)
    , length_(length) { }
  static inline word size_in_memory(word chars);
  inline array<code_point> chars();
  word length() { return length_; }
private:
  word length_;
};

class StringDescriptor : public DescriptorImpl<String, StringDescriptor> {
public:
  StringDescriptor(Descriptor *meta)
    : DescriptorImpl<String, StringDescriptor>(meta, tString) { }
  virtual allocation<Object> clone_object(Object *obj, Space &space);
  virtual word size_in_memory(Object *obj);
};

/* --- A r r a y --- */

class Array : public Object {

};

/* --- S i g n a l --- */

class ForwardPointer : public Data {
public:
  inline Object *target();
  static inline ForwardPointer *make(Object *value);
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
