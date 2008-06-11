#include <stdio.h>
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"

namespace plankton {

class FileNativesChannel : public neutrino::IExternalChannel {
public:
  virtual Value receive(neutrino::IMessage &value);
};


static Value open_file(IBuilder &factory, Value name_val) {
  const char *name = cast<String>(name_val).c_str();
  FILE *file = stdc_fopen(name, "r");
  if (file == NULL) return factory.get_null();
  Proxy<FILE*> buffer = factory.new_proxy<FILE*>();
  buffer.set(file);
  return buffer;
}


static Value read_file(IBuilder &factory, Value file_obj) {
  FILE *file = cast< Proxy<FILE*> >(file_obj).get();
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


static Value close_file(IBuilder &factory, Value file_obj) {
  Proxy<FILE*> buf = cast< Proxy<FILE*> >(file_obj);
  FILE *file = buf.get();
  buf.set(NULL);
  fclose(file);
  return factory.get_null();
}


Value FileNativesChannel::receive(neutrino::IMessage &message) {
  Tuple args = cast<Tuple>(message.contents());
  IBuilder &factory = message.context().factory();
  int operation = cast<Integer>(args[0]).value();
  switch (operation) {
    case 0: return open_file(factory, args[1]);
    case 1: return read_file(factory, args[1]);
    case 2: return close_file(factory, args[1]);
    default: return factory.get_null();
  }
}

SETUP_NEPTUNE_CHANNEL(file_natives)(neutrino::IExternalChannelConfiguration &config) {
  FileNativesChannel *channel = new FileNativesChannel();
  config.bind(*channel);
}

} // neutrino
