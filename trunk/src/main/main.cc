#include <stdio.h>

#include "io/image.h"
#include "main/options.h"
#include "monitor/monitor.h"
#include "platform/abort.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"
#include "utils/flags.h"
#include "utils/scoped-ptrs-inl.h"
#include "utils/vector.h"

namespace neutrino {

class Main {
public:
  static void main(list<char*> &args);
  static void on_option_error(string message);
  static Image *read_image(string name);
  static DynamicLibraryCollection *load_dynamic_libraries();
};

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
      Conditions::get().error_occurred("Error loading library %s", libs[i].chars());
      return NULL;
    }
  }
  return result.release();
}

void Main::main(list<char*> &args) {
  if (!Abort::setup_signal_handler()) return;
  FlagParser::parse_flags(args, on_option_error);
  own_ptr<DynamicLibraryCollection> dylibs(load_dynamic_libraries());
  if (*dylibs == NULL) return;
  list<string> files = Options::images;
  Runtime runtime(*dylibs);
  runtime.initialize();
  Runtime::Scope runtime_scope(runtime);
  for (uword i = 0; i < files.length(); i++) {
    RefScope ref_scope;
    string file = files[i];
    Image *image = read_image(file);
    bool loaded = runtime.load_image(*image);
    USE(loaded); ASSERT(loaded);
    delete image;
  }
  runtime.start();
  if (Options::print_stats_on_exit) {
    string_buffer stats;
    Monitor::write_on(stats);
    printf("%s", stats.to_string().chars());
  }
}

/**
 * Reads the contents of the specified file into a string.
 */
Image *Main::read_image(string name) {
  FILE *file = fopen(name.chars(), "rb");
  if (file == NULL) {
    printf("Unable to open %s.\n", name.chars());
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  uword size = ftell(file);
  rewind(file);
  uword *buffer = new uword[size];
  for (uword i = 0; i < size / kWordSize;) {
    const uword kSize = 256;
    uint8_t bytes[kSize];
    uword count = fread(bytes, kWordSize, kSize / kWordSize, file);
    if (count <= 0) {
      fclose(file);
      delete[] buffer;
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
  return new Image(size, buffer);
}

}

/**
 * Entry point.
 */
int main(int argc, char *argv[]) {
  neutrino::list<char*> args(argv, argc);
  neutrino::Main::main(args);
  return 0;
}
