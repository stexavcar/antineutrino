#include "heap/ref-inl.h"
#include "main/start.h"
#include "platform/stdc-inl.h"
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"
#include "runtime/runtime.h"
#include "utils/log.h"
#include "values/values-inl.pp.h"
#include "whiteout/cairo-backend.h"


using namespace neutrino;
using namespace plankton;


namespace whiteout {


class CairoProxy : public neutrino::MappingObjectProxy {
public:
  CairoProxy(CairoBackend &backend) : backend_(backend) { }
private:
  CairoBackend &backend() { return backend_; }
  CairoBackend &backend_;
};


class RuntimeThread : public NativeThread {
public:
  RuntimeThread(list<char*> &args) : args_(args) { }
  virtual likely run();
  void stop();
private:
  list<char*> &args() { return args_; }
  Runtime &runtime() { return runtime_; }
  list<char*> &args_;
  Runtime runtime_;
};

likely RuntimeThread::run() {
  @try(likely) Library::initialize_runtime(args(), runtime());
  RunMain run_main;
  runtime().schedule_async(run_main);
  @try(likely) runtime().start(false);
  return Success::make();
}

void RuntimeThread::stop() {
  runtime().schedule_interrupt();
}


static CairoBackend *cairo_backend;
SETUP_NEUTRINO_CHANNEL(whiteout)(neutrino::IProxyConfiguration &config) {
  CairoProxy *proxy = new CairoProxy(*cairo_backend);
  neutrino::MappingObjectProxyDescriptor &desc = proxy->descriptor();
  config.bind(*proxy);
}


likely main(list<char*> args) {
  Log::set_verbosity(Log::INFO);
  CairoBackend backend;
  cairo_backend = &backend;
  RuntimeThread thread(args);
  thread.start();
  backend.initialize(640, 480);
  backend.open();
  return thread.join();
}


} // namespace whiteout


int main(int argc, char *argv[]) {
  list<char*> args(argv, argc);
  likely result = whiteout::main(args);
  return Library::report_result(result);
}
