#ifndef _MAIN_NEUTRINO
#define _MAIN_NEUTRINO

#include <stdio.h>

#include "io/read.h"
#include "plankton/plankton-inl.h"
#include "utils/log.h"
#include "utils/vector-inl.h"
#include "utils/flags.h"
#include "utils/smart-ptrs-inl.h"

namespace neutrino {

class MainOptions : public OptionCollection {
public:
  MainOptions() {
    inherit(Log::options());
    REGISTER_FLAG(files, vector<string>());
  }
  vector<string> files() { return files_; }
private:
  vector<string> files_;
};


template <> class resource_cleaner<FILE*> {
public:
  static void release(FILE *file) {
    if (file != NULL)
      fclose(file);
  }
};


static string read_file(string name) {
  own_resource<FILE*> file_resource(fopen(name.start(), "rb"));
  FILE *file = *file_resource;
  if (file == NULL) {
    LOG().error("Unable to open %.\n", vargs(name));
    return string();
  }
  fseek(file, 0, SEEK_END);
  word size = ftell(file);
  rewind(file);
  own_vector<uint8_t> buffer(vector<uint8_t>::allocate(size));
  uint8_t *cursor = buffer.start();
  for (word i = 0; i < size / kWordSize;) {
    word count = fread(cursor, 1, size, file);
    if (count <= 0) {
      fclose(file);
      return string();
    }
    size -= count;
    cursor += count;
  }
  vector<uint8_t> data = buffer.release();
  return string(reinterpret_cast<char*>(data.start()), data.length());
}




void main(vector<const char*> args) {
  Abort::install_signal_handlers();
  MainOptions options;
  vector<string> values = options.parse(args);
  for (word i = 0; i < options.files().length(); i++) {
    string file = read_file(options.files()[i]);
    assert !file.is_empty();
    Factory factory;
    p::Value value = Reader::read(factory, file.start());
    assert !value.is_empty();
    string_stream stream;
    stream.add("%", vargs(value));
    stream.raw_c_str().println();
  }
}

} // namespace neutrino

int main(int argc, const char *argv[]) {
  neutrino::main(neutrino::vector<const char*>(argv, argc));
}

#endif // _MAIN_NEUTRINO
