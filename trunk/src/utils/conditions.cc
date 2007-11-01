#include "heap/values-inl.h"
#include "utils/checks.h"

namespace neutrino {

static Conditions kDefaultConditions;
Conditions *Conditions::current_ = NULL;

Conditions &Conditions::get() {
  if (current_ == NULL) return kDefaultConditions;
  else return *current_;
}

void Conditions::notify(Condition cause) {
  // ignore
}

MAKE_ENUM_INFO_HEADER(Condition)
FOR_EACH_CONDITION(MAKE_ENUM_INFO_ENTRY)
MAKE_ENUM_INFO_FOOTER()

void Conditions::check_is_failed(const char *file_name, int line_number,
    const char *type_name, uint32_t type_tag, Data *data,
    const char *value_source, Condition cause) {
  notify(cause);
#ifdef DEBUG
  static const char *kErrorMessage =
    "#\n"
    "# %s:%i: CHECK_IS(%s, %s) failed\n"
    "#   expected: %s\n"
    "#   found: %s\n"
    "#\n";
  EnumInfo<InstanceType> enum_info;
  const char *expected_name = Class::class_name(type_tag);
  uint32_t value_tag = Class::tag_of(data);
  const char *value_type_name = Class::class_name(value_tag);
  printf(kErrorMessage, file_name, line_number, type_name, value_source,
      expected_name, value_type_name);
#else // DEBUG
  static const char *kErrorMessage =
    "#\n"
    "# %s:%i: CHECK_IS(%s, %s) failed\n"
    "#\n";
  printf(kErrorMessage, file_name, line_number, type_name, value_source);
#endif // DEBUG  
  abort();
}

void Conditions::check_failed(const char *file_name, int line_number,
    const char *source, bool value, Condition cause) {
  notify(cause);
  static const char *kErrorMessage =
    "#\n"
    "# %s:%i: CHECK(%s) failed\n"
    "#\n";
  printf(kErrorMessage, file_name, line_number, source);
  abort();
}

void Conditions::check_eq_failed(const char* file_name, int line_number,
    int expected, const char *expected_source, int value,
    const char *value_source, Condition cause) {
  notify(cause);
  static const char *kErrorMessage =
    "#\n"
    "# %s:%i: CHECK_EQ(%s, %s) failed\n"
    "#   expected: %i\n"
    "#   found: %i\n"
    "#\n";
  printf(kErrorMessage, file_name, line_number, expected_source,
      value_source, expected, value);
  abort();
}

void Conditions::check_eq_failed(const char* file_name, int line_number,
    Value *expected, const char *expected_source, Value *value,
    const char *value_source, Condition cause) {
  notify(cause);
  static const char *kErrorMessage =
    "#\n"
    "# %s:%i: CHECK_EQ(%s, %s) failed\n"
    "#   expected: %x\n"
    "#   found: %x\n"
    "#\n";
  printf(kErrorMessage, file_name, line_number, expected_source,
      value_source, reinterpret_cast<word>(expected),
      reinterpret_cast<word>(value));
  abort();
}

void Conditions::unreachable(const char *file_name, int line_number,
    Condition cause) {
  notify(cause);
  static const char *kErrorMessage =
    "#\n"
    "# %s:%i: Unreachable code\n"
    "#\n";
  printf(kErrorMessage, file_name, line_number);
  abort();
}

void Conditions::unhandled(const char *file_name, int line_number,
    const char *enum_name, int32_t value, AbstractEnumInfo &info,
    Condition cause) {
  notify(cause);
  string name = info.get_name_for(value);
  static const char *kErrorMessage =
    "#\n"
    "# %s:%i: Unhandled %s value %s\n"
    "#\n";
  printf(kErrorMessage, file_name, line_number, enum_name, name.chars());
  abort();
}

void Conditions::error_occurred(const char *format) {
  printf(format);
  printf("\n");
  exit(0);
}

} // namespace neutrino
