#ifndef _VALUES_METHOD
#define _VALUES_METHOD

namespace neutrino {

// -------------------
// --- M e t h o d ---
// -------------------

#define FOR_EACH_METHOD_FIELD(VISIT, arg)                            \
  VISIT(String,    name,      Name,      arg)                        \
  VISIT(Signature, signature, Signature, arg)                        \
  VISIT(Lambda,    lambda,    Lambda,    arg)

class Method : public Object {
public:
  FOR_EACH_METHOD_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kNameOffset      = Object::kHeaderSize;
  static const uint32_t kSignatureOffset = kNameOffset + kPointerSize;
  static const uint32_t kLambdaOffset    = kSignatureOffset + kPointerSize;
  static const uint32_t kSize            = kLambdaOffset + kPointerSize;
};

template <> class ref_traits<Method> : public ref_traits<Object> {
public:
  FOR_EACH_METHOD_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Method);


// -------------------------
// --- S i g n a t u r e ---
// -------------------------

#define FOR_EACH_SIGNATURE_FIELD(VISIT, arg)                         \
  VISIT(Tuple, parameters, Parameters, arg)

class Signature : public Object {
public:
  FOR_EACH_SIGNATURE_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uint32_t kParametersOffset = Object::kHeaderSize;
  static const uint32_t kSize             = kParametersOffset + kPointerSize;
};

template <> class ref_traits<Signature> : public ref_traits<Object> {
public:
  FOR_EACH_SIGNATURE_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Signature);


} // neutrino

#endif // _VALUES_METHOD
#ifndef _VALUES_METHOD
#define _VALUES_METHOD

namespace neutrino {

// -------------------
// --- M e t h o d ---
// -------------------

#define FOR_EACH_METHOD_FIELD(VISIT, arg)                            \
  VISIT(String,    name,      Name,      arg)                        \
  VISIT(Signature, signature, Signature, arg)                        \
  VISIT(Lambda,    lambda,    Lambda,    arg)

class Method : public Object {
public:
  FOR_EACH_METHOD_FIELD(DECLARE_OBJECT_FIELD, 0)
  
  static const uint32_t kNameOffset      = Object::kHeaderSize;
  static const uint32_t kSignatureOffset = kNameOffset + kPointerSize;
  static const uint32_t kLambdaOffset    = kSignatureOffset + kPointerSize;
  static const uint32_t kSize            = kLambdaOffset + kPointerSize;
};

template <> class ref_traits<Method> : public ref_traits<Object> {
public:
  FOR_EACH_METHOD_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Method);


// -------------------------
// --- S i g n a t u r e ---
// -------------------------

#define FOR_EACH_SIGNATURE_FIELD(VISIT, arg)                         \
  VISIT(Tuple, parameters, Parameters, arg)

class Signature : public Object {
public:
  FOR_EACH_SIGNATURE_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uint32_t kParametersOffset = Object::kHeaderSize;
  static const uint32_t kSize             = kParametersOffset + kPointerSize;
};

template <> class ref_traits<Signature> : public ref_traits<Object> {
public:
  FOR_EACH_SIGNATURE_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Signature);


} // neutrino

#endif // _VALUES_METHOD
