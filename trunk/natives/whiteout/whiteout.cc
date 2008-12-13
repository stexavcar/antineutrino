#include "heap/ref-inl.h"
#include "main/start.h"
#include "platform/stdc-inl.h"
#include "plankton/neutrino.h"
#include "platform/stdc-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.pp.h"
#include "whiteout/cairo-quartz.h"


using namespace neutrino;


namespace whiteout {


class CairoProxy : public neutrino::MappingObjectProxy {
public:
  CairoProxy();
  plankton::Value init(neutrino::IMessage &message);
private:
};


CairoProxy::CairoProxy() {

}


plankton::Value CairoProxy::init(neutrino::IMessage &message) {
  return message.context().factory().get_null();
}


SETUP_NEUTRINO_CHANNEL(whiteout)(neutrino::IProxyConfiguration &config) {
  CairoProxy *proxy = new CairoProxy();
  neutrino::MappingObjectProxyDescriptor &desc = proxy->descriptor();
  desc.register_method("init", 0, &CairoProxy::init);
  config.bind(*proxy);
}


likely main(neutrino::list<char*> args) {
  Runtime runtime;
  @try(likely) Library::initialize_runtime(args, runtime);
  ref_block<> protect(runtime.refs());
  ref<HashMap> toplevel(&runtime.roots().toplevel());
  @check(probably) ref<String> main_name = runtime.factory().new_string("entry_point");
  Data *entry_point = runtime.roots().toplevel()->get(*main_name);
  @try(likely) Library::run(runtime);
  return Success::make();
}


} // namespace whiteout


int main(int argc, char *argv[]) {
  list<char*> args(argv, argc);
  likely result = whiteout::main(args);
  return Library::report_result(result);
}
