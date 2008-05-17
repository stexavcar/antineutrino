#include <stdio.h>
#include "public/neutrino.h"
#include "platform/stdc-inl.h"

namespace neutrino {

class FileNativesChannel : public IExternalChannel {
public:
  virtual NValue receive(IMessage &value);
};


static NValue open_file(IValueFactory &factory, NValue name_val) {
  const char *name = cast<NString>(name_val).c_str();
  FILE *file = stdc_fopen(name, "r");
  if (file == NULL) return factory.get_null();
  NProxy<FILE*> buffer = factory.new_proxy<FILE*>();
  buffer.set(file);
  return buffer;
}


static NValue read_file(IValueFactory &factory, NValue file_obj) {
  FILE *file = cast< NProxy<FILE*> >(file_obj).get();
  fseek(file, 0, SEEK_END);
  unsigned size = ftell(file);
  rewind(file);
  char *result = new char[size + 1];
  unsigned offset = 0;
  while (offset < size) {
    const unsigned kSize = 256;
    unsigned char bytes[kSize];
    unsigned count = fread(bytes, 1, kSize, file);
    for (unsigned i = 0; i < count; i++, offset++)
      result[offset] = bytes[i];
  }
  result[size] = '\0';
  return factory.new_string(result, size);
}


static NValue close_file(IValueFactory &factory, NValue file_obj) {
  NProxy<FILE*> buf = cast< NProxy<FILE*> >(file_obj);
  FILE *file = buf.get();
  buf.set(NULL);
  fclose(file);
  return factory.get_null();
}


NValue FileNativesChannel::receive(IMessage &message) {
  NTuple args = cast<NTuple>(message.contents());
  IValueFactory &factory = message.context().factory();
  int operation = cast<NInteger>(args[0]).value();
  switch (operation) {
    case 0: return open_file(factory, args[1]);
    case 1: return read_file(factory, args[1]);
    case 2: return close_file(factory, args[1]);
    default: return factory.get_null();
  }
}

SETUP_NEPTUNE_CHANNEL(file_natives)(IExternalChannelConfiguration &config) {
  FileNativesChannel *channel = new FileNativesChannel();
  config.bind(*channel);
}

} // neutrino
