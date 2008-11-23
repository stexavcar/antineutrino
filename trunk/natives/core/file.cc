#include <stdio.h>
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"

namespace plankton {

class FileNativesChannel : public neutrino::MappingObjectProxy {
public:
  Value open(neutrino::IMessage &message);
  Value read(neutrino::IMessage &message);
  Value close(neutrino::IMessage &message);
};


Value FileNativesChannel::open(neutrino::IMessage &message) {
  const char *name = cast<String>(message.contents()).c_str();
  FILE *file = stdc_fopen(name, "r");
  if (file == NULL) return message.context().factory().get_null();
  Proxy<FILE*> buffer = message.context().factory().new_proxy<FILE*>();
  buffer.set(file);
  return buffer;
}


Value FileNativesChannel::read(neutrino::IMessage &message) {
  FILE *file = cast< Proxy<FILE*> >(message.contents()).get();
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
  return message.context().factory().new_string(result, size);
}


Value FileNativesChannel::close(neutrino::IMessage &message) {
  Proxy<FILE*> buf = cast< Proxy<FILE*> >(message.contents());
  FILE *file = buf.get();
  buf.set(NULL);
  fclose(file);
  return message.context().factory().get_null();
}


SETUP_NEUTRINO_CHANNEL(file_natives)(neutrino::IProxyConfiguration &config) {
  FileNativesChannel *channel = new FileNativesChannel();
  neutrino::MappingObjectProxyDescriptor &desc = channel->descriptor();
  desc.register_method("open", &FileNativesChannel::open);
  desc.register_method("read", &FileNativesChannel::read);
  desc.register_method("close", &FileNativesChannel::close);
  config.bind(*channel);
}

} // neutrino
