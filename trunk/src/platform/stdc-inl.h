#ifndef _PLATFORM_STDC_INL
#define _PLATFORM_STDC_INL

#include <stdarg.h>

/**
 * This file redefines all the standard c functions that must or
 * should be invoked differently on some platforms.  It is an inline
 * file so that native channels can use it.
 */

namespace neutrino {

#ifdef MSVC

static inline FILE *stdc_fopen(const char *filename, const char *modes) {
  FILE *result = NULL;
  ::fopen_s(&result, filename, modes);
  return result;
}

static inline int stdc_snprintf(char *str, size_t size, const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  int result = ::_vsnprintf_s(str, size, size, format, ap);
  va_end(ap);
  return result;
}

#else

#define stdc_fopen fopen
#define stdc_snprintf snprintf

#endif

} // neutrino


#endif // _PLATFORM_STDC_INL
