#include "platform/spawn.h"


namespace positron {

class DummySocket : public ISocket {
public:
  virtual word read(vector<uint8_t> &data) { return 0; }
  virtual word write(const vector<uint8_t> &data) { return 0; }
};

ISocket &ParentProcess::socket() {
  return *new DummySocket();
}

bool ParentProcess::open() {
  return false;
}

ISocket &ChildProcess::socket() {
  return *new DummySocket();
}

bool ChildProcess::open(string &command, vector<string> &args,
    vector< pair<string> > &env) {
  return false;
}

word ChildProcess::wait() {
  return -1;
}

} // namespace positron
