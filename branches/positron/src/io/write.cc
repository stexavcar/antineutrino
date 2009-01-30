#include "io/miniheap-inl.h"
#include "io/write.h"

namespace neutrino {

class WriteService {
public:
  WriteService();
  p::Value unparse(Message &message);
  void unparse_on(p::Value val, string_stream &stream);
  ObjectProxyDTable<WriteService> &dtable() { return dtable_; }
private:
  ObjectProxyDTable<WriteService> dtable_;
};

static p::Object create_write() {
  static WriteService *instance = NULL;
  if (instance == NULL)
    instance = new WriteService();
  return instance->dtable().proxy_for(*instance);
}

REGISTER_SERVICE(neutrino.io.write, create_write);

void Write::load() { }

WriteService::WriteService() {
  dtable().add_method("unparse", &WriteService::unparse);
}

p::Value WriteService::unparse(Message &message) {
  assert message.args().length() == 1;
  string_stream stream;
  unparse_on(message.args()[0], stream);
  Factory *factory = new Factory();
  message.data()->acquire_resource(*factory);
  return factory->new_string(stream.raw_c_str());
}

void WriteService::unparse_on(p::Value value, string_stream &stream) {
  switch (value.type()) {
    case p::Value::vtNull:
      stream.add("null");
      break;
    case p::Value::vtArray: {
      p::Array array = cast<p::Array>(value);
      word length = array.length();
      stream.add("(");
      for (word i = 0; i < length; i++) {
        if (i > 0) stream.add(" ");
        unparse_on(array[i], stream);
      }
      stream.add(")");
      break;
    }
    case p::Value::vtInteger: {
      word value = cast<p::Integer>(value).value();
      stream.add("%", args(value));
      break;
    }
    case p::Value::vtString: {
      p::String str = cast<p::String>(value);
      for (word i = 0; str[i]; i++)
        stream.add(str[i]);
      break;
    }
    case p::Value::vtSeed: {
      assert false;
      break;
    }
    case p::Value::vtObject:
      stream.add("#<an Object>");
      break;
  }
}

} // namespace neutrino
