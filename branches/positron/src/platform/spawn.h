#ifndef _PLATFORM_SOCKET
#define _PLATFORM_SOCKET

#include "plankton/builder.h"
#include "platform/abort.h"
#include "utils/global.h"
#include "utils/string.h"
#include "utils/smart-ptrs.h"

namespace positron {

class ParentProcess {
public:
  class Data;
  ParentProcess() : data_(NULL) { }
  p_object proxy();
  bool open();
  bool receive(MessageIn &message);
private:
  Data *data() { return data_; }
  Data *data_;
};

class ChildProcess {
public:
  class Data;
  ChildProcess()
    : data_(NULL) { }
  p_object proxy();
  bool open(string &command, vector<string> &args, vector< pair<string> > &env);
  word wait();
  bool receive(MessageIn &message);
private:
  Data *data() { return data_; }
  Data *data_;
};

} // namespace positron

#endif // _PLATFORM_SOCKET
