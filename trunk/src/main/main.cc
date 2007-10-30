#include <stdio.h>

#include "io/parser.h"
#include "io/sexp.h"
#include "runtime/runtime-inl.h"
#include "utils/list-inl.h"

using namespace neutrino;

class Main {
public:
  static void main(list<char*> &args);
  static string read_file(char *name);
};

void Main::main(list<char*> &args) {
  Runtime runtime;
  runtime.initialize();
  Runtime::Scope runtime_scope(runtime);
  Parser parser(runtime);
  for (uint32_t i = 1; i < args.length(); i++) {
    RefScope ref_scope;
    char *arg = args[i];
    string contents = read_file(arg);
    ref<String> str = runtime.factory().new_string(contents);
    contents.dispose();
    zone::Zone zone;
    ProgramSymbolResolver resolver;
    sexp::Sexp *tree = sexp::Reader::read(str, resolver);
    if (tree == NULL) {
      printf("Error reading program\n");
      exit(1);
    }
    sexp::List &program = sexp::cast<sexp::List>(*tree);
    parser.load(program);
  }
  runtime.start();
}

/**
 * Reads the contents of the specified file into a string.
 */
string Main::read_file(char *name) {
  FILE *file = fopen(name, "rb");
  if (file == NULL) {
    printf("Unable to open %s.\n", name);
    exit(1);
  }
  fseek(file, 0, SEEK_END);
  uint32_t size = ftell(file);
  rewind(file);
  char *buffer = new char[size + 1];
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
  return string(buffer, size);
}

/**
 * Entry point.
 */
int main(int argc, char *argv[]) {
  list<char*> args(argv, argc);
  Main::main(args);
  return 0;
}
