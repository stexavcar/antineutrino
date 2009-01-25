#ifndef _PLATFORM_SOCKET
#define _PLATFORM_SOCKET

#include "io/miniheap.h"
#include "platform/abort.h"
#include "utils/global.h"
#include "utils/string.h"
#include "utils/smart-ptrs.h"
#include "value/condition-inl.h"

namespace neutrino {

class ParentProcess {
public:
  class Data;
  ParentProcess() : data_(NULL) { }
  ~ParentProcess();
  p::Object proxy();
  boole open();
  boole receive(MessageIn &message);
private:
  Data *data() { return *data_; }
  own_ptr<Data> data_;
};

class ChildProcess {
public:
  class Data;
  ChildProcess() : data_(NULL) { }
  ~ChildProcess();
  p::Object proxy();
  boole open(string &command, vector<string> &args, vector< pair<string> > &env);
  word wait();
  boole receive(MessageIn &message);
private:
  Data *data() { return *data_; }
  own_ptr<Data> data_;
};

} // namespace neutrino

#endif // _PLATFORM_SOCKET
