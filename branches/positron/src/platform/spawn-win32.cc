#include "platform/spawn.h"
#include "utils/log.h"
#include "utils/string-inl.h"
#include <windows.h>


namespace neutrino {

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
  STARTUPINFOA si_startup_info;
  PROCESS_INFORMATION pi_process_info;
  memset(&si_startup_info, 0, sizeof(si_startup_info));
  memset(&pi_process_info, 0, sizeof(pi_process_info));
  si_startup_info.cb = sizeof(si_startup_info);
  if (!CreateProcess(command.start(), 0, 0, 0, true, 0, 0, 0,
        &si_startup_info, &pi_process_info)) {
    LOG().error("Error creating process (%)", neutrino::args(GetLastError()));
    return false;
  }
  return true;
}

word ChildProcess::wait() {
  return -1;
}

} // namespace neutrino
