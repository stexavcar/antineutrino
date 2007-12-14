#ifndef _UTILS_NONLOCAL
#define _UTILS_NONLOCAL

#include <setjmp.h>

/**
 * \file
 * Abstraction for nonlocal escapes; a primitive exception mechanism.
 * Basically a wrapper around \c setjmp and \c longjmp.
 */

namespace neutrino {

/**
 * An object that allows nonlocal escape.  A NonLocal is can be used
 * together with one use of TRY() / CATCH(), and a call to do_throw()
 * on that NonLocal will cause control to escape back to the site of
 * the TRY() / CATCH() macros.
 */
class NonLocal {
 public:
  NonLocal();
  jmp_buf &jump_buffer() { return jump_buffer_; }
  void do_throw(int value);
  static const int kNoException = 0;
 private:
  jmp_buf jump_buffer_;
};

#define TRY(non_local) do {                                          \
  int __exception__ = setjmp((non_local).jump_buffer());             \
  if (__exception__ == NonLocal::kNoException) {

#define CATCH(cases) } else {                                        \
      switch (__exception__) { cases }                               \
    }                                                                \
  } while (false);  

#define TRY_CATCH_EXCEPTION() __exception__

}

#endif // _UTILS_NONLOCAL
