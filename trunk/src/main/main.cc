#include <stdio.h>

#include "io/image.h"
#include "main/options.h"
#include "monitor/monitor.h"
#include "platform/abort.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"
#include "utils/flags.h"
#include "utils/vector.h"

namespace neutrino {

class Main {
public:
  static void main(list<char*> &args);
  static void on_option_error(string message);
  static Image *read_image(string name);
};

void Main::on_option_error(string message) {
  message.println();
  exit(1);
}

void Main::main(list<char*> &args) {
  if (!Abort::setup_signal_handler()) return;
  FlagParser::parse_flags(args, on_option_error);
  list<string> files = Options::images;
  Runtime runtime;
  runtime.initialize();
  Runtime::Scope runtime_scope(runtime);
  for (uint32_t i = 0; i < files.length(); i++) {
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
  uint32_t size = ftell(file);
  rewind(file);
  uint32_t *buffer = new uint32_t[size];
  for (uint32_t i = 0; i < size / kWordSize;) {
    const uint32_t kSize = 256;
    uint8_t bytes[kSize];
    uint32_t count = fread(bytes, kWordSize, kSize / kWordSize, file);
    if (count <= 0) {
      fclose(file);
      delete[] buffer;
      return NULL;
    }
    for (uint32_t j = 0; j < count; j++) {
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
