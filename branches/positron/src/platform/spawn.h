#ifndef _PLATFORM_SPAWN
#define _PLATFORM_SPAWN

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
  ParentProcess();
  ~ParentProcess();
  p::Object proxy();
  possibly open();
  possibly receive(MessageIn &message);
private:
  Data *data() { return *data_; }
  own_ptr<Data> data_;
};

class ChildProcess {
public:
  class Data;
  ChildProcess();
  ~ChildProcess();
  p::Object proxy();
  possibly open(string &command, vector<string> &args,
      vector< pair<string, string> > &env);
  word wait();
  possibly receive(MessageIn &message);
private:
  Data *data() { return *data_; }
  own_ptr<Data> data_;
};

} // namespace neutrino

#endif // _PLATFORM_SPAWN
