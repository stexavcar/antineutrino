#include <stdio.h>

#include "io/image.h"
#include "io/parser.h"
#include "io/sexp.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"

using namespace neutrino;

class Main {
public:
  static void main(list<char*> &args);
  static Image *read_image(char *name);
};

void Main::main(list<char*> &args) {
  Runtime runtime;
  runtime.initialize();
  Runtime::Scope runtime_scope(runtime);
  Parser parser(runtime);
  for (uint32_t i = 1; i < args.length(); i++) {
    RefScope ref_scope;
    char *arg = args[i];
    Image *image = read_image(arg);
    if (!image->reset()) {
      printf("Invalid image %s.\n", arg);
      exit(1);
    }
    ASSERT(runtime.load_image(*image));
  }
  runtime.start();
}

/**
 * Reads the contents of the specified file into a string.
 */
Image *Main::read_image(char *name) {
  FILE *file = fopen(name, "rb");
  if (file == NULL) {
    printf("Unable to open %s.\n", name);
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  uint32_t size = ftell(file);
  rewind(file);
  uint8_t *buffer = new uint8_t[size + 1];
  buffer[size] = '\0';
  for (uint32_t i = 0; i < size;) {
    uint32_t count = fread(buffer + i, 1, size - i, file);
    if (count <= 0) {
      fclose(file);
      free(buffer);
      return NULL;
    }
    i += count;
  }
  fclose(file);
  return new Image(size, buffer);
}

/**
 * Entry point.
 */
int main(int argc, char *argv[]) {
  list<char*> args(argv, argc);
  Main::main(args);
  return 0;
}
