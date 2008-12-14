#include "main/start.h"

int main(int argc, char *argv[]) {
  neutrino::list<char*> args(argv, argc);
  return neutrino::Library::main(args);
}
