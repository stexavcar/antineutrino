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
  ISocket &socket();
  bool open();
private:
  Data *data() { return data_; }
  Data *data_;
};

class ChildProcess {
public:
  class Data;
  ChildProcess(string &command, vector<string> &args,
      vector< pair<string> > &env)
    : data_(NULL), command_(command), args_(args), env_(env) { }
  ISocket &socket();
  bool open();
  word wait();
private:
  Data *data() { return data_; }
  string &command() { return command_; }
  vector<string> &args() { return args_; }
  vector< pair<string> > &env() { return env_; }
  Data *data_;
  string &command_;
  vector<string> &args_;
  vector< pair<string> > env_;
};

} // namespace positron

#endif // _PLATFORM_SOCKET
