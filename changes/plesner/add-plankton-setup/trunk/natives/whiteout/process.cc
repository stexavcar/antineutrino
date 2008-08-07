#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"

extern char **environ;

namespace plankton {

class ForkProcessChannel : public neutrino::IExternalChannel {
public:
  virtual Value receive(neutrino::IMessage &value);
};

Value ForkProcessChannel::receive(neutrino::IMessage &message) {
  pid_t pid = fork();
  if (pid != 0) return message.context().factory().get_null();
  Tuple argv_obj = cast<Tuple>(message.contents());
  int argc = argv_obj.length();
  const char **argv = new const char*[argc];
  for (int i = 0; i < argc; i++) {
    String arg = cast<String>(argv_obj[i]);
    argv[i] = arg.c_str();
  }
  execve(argv[0], const_cast<char*const*>(argv), environ);
  perror("Execve error");
  exit(0);
}

SETUP_NEUTRINO_CHANNEL(fork_process)(neutrino::IExternalChannelConfiguration &config) {
  ForkProcessChannel *channel = new ForkProcessChannel();
  config.bind(*channel);
}

} // neutrino
