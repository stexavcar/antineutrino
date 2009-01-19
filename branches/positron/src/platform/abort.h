#ifndef _PLATFORM_ABORT
#define _PLATFORM_ABORT

#include "utils/global.h"
#include "utils/string.h"

namespace positron {

class Finalized {
public:
  Finalized();
  virtual ~Finalized();
  virtual void cleanup() = 0;
  void install();
  void uninstall();
  bool has_been_finalized() { return has_been_finalized_; }
private:
  friend class Abort;
  bool has_been_finalized_;
  static Finalized *first_;
  Finalized *prev_;
  Finalized *next_;
};

class Abort {
public:
  static void abort(string format, const var_args &args);
  static void install_signal_handlers();
  static void finalize();
private:
  friend class Finalized;
  static bool has_cleaned_up_;
  static Finalized *first_;
  static Finalized *last_;
};

} // namespace positron

#endif // _PLATFORM_ABORT
