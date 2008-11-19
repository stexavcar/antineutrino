#include <stdio.h>

#include "heap/ref-inl.h"
#include "io/image.h"
#include "main/main.h"
#include "main/options.h"
#include "monitor/monitor.h"
#include "platform/abort.h"
#include "platform/stdc-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"
#include "utils/flags.h"
#include "utils/smart-ptrs-inl.h"
#include "utils/array.h"

namespace neutrino {

void Main::on_option_error(string message) {
  message.println();
  exit(1);
}

DynamicLibraryCollection *Main::load_dynamic_libraries() {
  // To begin with the result is owned by this scope so it will be
  // correctly cleaned up on premature return.
  own_ptr<DynamicLibraryCollection> result(&DynamicLibraryCollection::create());
  list<string> &libs = Options::libs;
  for (uword i = 0; i < libs.length(); i++) {
    if (!result->load(libs[i])) {
      Conditions::get().error_occurred("Error loading library %", elms(libs[i]));
      return NULL;
    }
  }
  return result.release();
}


int Main::main(list<char*> &args) {
  likely result = run_system(args);
  if (result.has_failed()) {
    FatalError *error = result.signal();
    string_buffer buf;
    buf.printf("Aborting: %", elms(error));
    buf.raw_string().println(stdout);
    return 1;
  } else {
    return 0;
  }
}


likely Main::run_system(list<char*> &args) {
  if (!Abort::setup_signal_handler()) return FatalError::make(FatalError::feInitialization);
  FlagParser::parse_flags(args, on_option_error);
  own_ptr<DynamicLibraryCollection> dylibs(load_dynamic_libraries());
  if (*dylibs == NULL) return FatalError::make(FatalError::feInitialization);
  list<string> files = Options::images;
  Runtime runtime(*dylibs);
  BytecodeArchitecture arch(runtime);
  @try(likely) runtime.initialize(&arch);
  for (uword i = 0; i < files.length(); i++) {
    string file = files[i];
    own_ptr<Image> image(read_image(file));
    @try(likely) runtime.load_image(**image);
  }
  @try(likely) build_arguments(runtime);
  likely start_result = runtime.start();
  if (Options::print_stats_on_exit) {
    string_buffer stats;
    Monitor::write_on(stats);
    stats.to_string().println();
  }
  return start_result;
}

likely Main::build_arguments(Runtime &runtime) {
  list<string> args = Options::args;
  ref_block<> protect(runtime.refs());
  @check(probably) ref<Tuple> result = runtime.factory().new_tuple(args.length());
  for (uword i = 0; i < args.length(); i++) {
    @check(probably) ref<String> arg = runtime.factory().new_string(args[i]);
    result->set(i, *arg);
  }
  @check(probably) ref<String> arguments = runtime.factory().new_string("arguments");
  runtime.gc_safe().set(runtime.toplevel(), arguments, result);
  return Success::make();
}


/**
 * Reads the contents of the specified file into a string.
 */
Image *Main::read_image(string name) {
  FILE *file = stdc_fopen(name.chars(), "rb");
  if (file == NULL) {
    Conditions::get().error_occurred("Unable to open %.\n", elms(name));
  }
  fseek(file, 0, SEEK_END);
  uword size = ftell(file);
  rewind(file);
  own_array<word> buffer(ALLOCATE_ARRAY(word, size));
  for (uword i = 0; i < size / kWordSize;) {
    const uword kSize = 256;
    uint8_t bytes[kSize];
    uword count = fread(bytes, kWordSize, kSize / kWordSize, file);
    if (count <= 0) {
      fclose(file);
      return NULL;
    }
    for (uword j = 0; j < count; j++) {
      uint8_t b0 = bytes[kWordSize * j + 0];
      uint8_t b1 = bytes[kWordSize * j + 1];
      uint8_t b2 = bytes[kWordSize * j + 2];
      uint8_t b3 = bytes[kWordSize * j + 3];
      buffer[i++] = (b0 << 0) | (b1 << 8) | (b2 << 16) | (b3 << 24);
    }
  }
  fclose(file);
  return new Image(list<word>(buffer.release().start(), size));
}

} // namespace neutrino
