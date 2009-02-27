#ifndef _MAIN_NEUTRINO
#define _MAIN_NEUTRINO

#include <stdio.h>

#include "code/ast.h"
#include "io/read.h"
#include "plankton/plankton-inl.h"
#include "runtime/gc-safe-inl.h"
#include "runtime/heap-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/log.h"
#include "utils/vector-inl.h"
#include "utils/flags.h"
#include "utils/smart-ptrs-inl.h"
#include "value/condition-inl.h"
#include "value/value.h"

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
  for (word i = 0; i < size;) {
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


void report_error(string name, string contents, Location loc) {
  // Scan ahead to the line that contains the error
  const char *current = contents.start();
  const char *raw_line = current;
  word line_no = 0;
  while (current != NULL) {
    if (current - contents.start() > loc.start)
      break;
    raw_line = current;
    line_no++;
    current = strchr(current, '\n');
    if (current != NULL) current++;
  }
  // Cut the line out if there is more source after it
  string line;
  if (current == NULL) line = string(raw_line);
  else line = string(raw_line, current - raw_line - 1);
  string_stream stream;
  stream.add("%:%: Parse error\n", vargs(name, line_no));
  stream.add("%\n", vargs(line));
  // Add underline
  for (word i = raw_line - contents.start(); i < loc.start; i++)
    stream.add(' ');
  for (word i = loc.start; i <= loc.end; i++)
    stream.add('^');
  stream.raw_string().println();
}

possibly start(vector<const char*> args) {
  Abort::install_signal_handlers();
  MainOptions options;
  vector<string> values = options.parse(args);
  for (word i = 0; i < options.files().length(); i++) {
    string file = read_file(options.files()[i]);
    assert !file.is_empty();
    Arena arena;
    SexpParser parser(file, arena);
    s_exp *expr = parser.read();
    if (expr == NULL) {
      Location loc = parser.error_location();
      report_error(options.files()[i], file, loc);
      return FatalError::make(FatalError::feAbort);
    }
    Runtime runtime;
    try runtime.initialize();
    CodeStream code(runtime);
    code.add(expr);
    string_stream str;
    protector<1> protect(runtime.refs());
    try alloc ref<Array> literals = code.literals();
    vector<code_t> instrs = code.data();
    CodeStream::disassemble(instrs, literals, str);
    str.raw_string().println();
  }
  return Success::make();
}

void main(vector<const char*> args) {
  possibly result = start(args);
  if (!result.has_succeeded()) {
    LOG().error("Execution error: %", vargs(result.failure()));
  }
}

} // namespace neutrino

int main(int argc, const char *argv[]) {
  neutrino::main(neutrino::vector<const char*>(argv, argc));
}

#endif // _MAIN_NEUTRINO
