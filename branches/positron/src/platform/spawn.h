#ifndef _PLATFORM_SOCKET
#define _PLATFORM_SOCKET

#include "plankton/builder.h"
#include "platform/abort.h"
#include "utils/global.h"
#include "utils/string.h"
#include "utils/smart-ptrs.h"
#include "value/condition-inl.h"

namespace positron {

class ParentProcess {
public:
  class Data;
  ParentProcess() : data_(NULL) { }
  ~ParentProcess();
  p_object proxy();
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
  p_object proxy();
  boole open(string &command, vector<string> &args, vector< pair<string> > &env);
  word wait();
  boole receive(MessageIn &message);
private:
  Data *data() { return *data_; }
  own_ptr<Data> data_;
};

} // namespace positron

#endif // _PLATFORM_SOCKET
