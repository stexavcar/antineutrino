#ifndef _VALUES_METHOD
#define _VALUES_METHOD

namespace neutrino {

// -------------------
// --- M e t h o d ---
// -------------------

#define FOR_EACH_METHOD_FIELD(VISIT, arg)                            \
  VISIT(Selector,  selector,  Selector,  arg)                        \
  VISIT(Signature, signature, Signature, arg)                        \
  VISIT(Lambda,    lambda,    Lambda,    arg)

class Method : public Object {
public:
  FOR_EACH_METHOD_FIELD(DECLARE_OBJECT_FIELD, 0)

  static const uword kSelectorOffset  = Object::kHeaderSize;
  static const uword kSignatureOffset = kSelectorOffset + kPointerSize;
  static const uword kLambdaOffset    = kSignatureOffset + kPointerSize;
  static const uword kSize            = kLambdaOffset + kPointerSize;
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

  static const uword kParametersOffset = Object::kHeaderSize;
  static const uword kSize             = kParametersOffset + kPointerSize;
};

template <> class ref_traits<Signature> : public ref_traits<Object> {
public:
  FOR_EACH_SIGNATURE_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Signature);


// -----------------------
// --- S e l e c t o r ---
// -----------------------

#define FOR_EACH_SELECTOR_FIELD(VISIT, arg)                          \
  VISIT(Immediate, name,        Name,       arg)                     \
  VISIT(Smi,       argc,        Argc,       arg)                     \
  VISIT(Tuple,     keywords,    Keywords,   arg)                     \
  VISIT(Bool,      is_accessor, IsAccessor, arg)

class Selector : public Object {
public:
  FOR_EACH_SELECTOR_FIELD(DECLARE_OBJECT_FIELD, 0)

  bool selector_equals(Selector *that);

  static const uword kNameOffset       = Object::kHeaderSize;
  static const uword kArgcOffset       = kNameOffset + kPointerSize;
  static const uword kKeywordsOffset   = kArgcOffset + kPointerSize;
  static const uword kIsAccessorOffset = kKeywordsOffset + kPointerSize;
  static const uword kSize             = kIsAccessorOffset + kPointerSize;
};

template <> class ref_traits<Selector> : public ref_traits<Object> {
public:
  FOR_EACH_SELECTOR_FIELD(DECLARE_REF_FIELD, 0)
};

DEFINE_REF_CLASS(Selector);


} // neutrino

#endif // _VALUES_METHOD
