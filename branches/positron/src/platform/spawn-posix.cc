#include "io/miniheap-inl.h"
#include "plankton/plankton-inl.h"
#include "platform/spawn.h"
#include "utils/check-inl.h"
#include "utils/smart-ptrs-inl.h"
#include "utils/log.h"
#include "utils/array-inl.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>

namespace neutrino {

static const string kMasterEnvVariable = "POSITRON_MASTER";

class SpawnService {
public:
  SpawnService();
  p::Value spawn(Message &message);
  static p::Object create();
private:
  ObjectProxyDTable<SpawnService> &dtable() { return dtable_; }
  ObjectProxyDTable<SpawnService> dtable_;
};

SpawnService::SpawnService() {
  dtable().add_method("spawn", &SpawnService::spawn);
}

p::Value SpawnService::spawn(Message &message) {
  assert message.args().length() == 3;
  return Factory::get_null();
}

p::Object SpawnService::create() {
  static SpawnService instance;
  return instance.dtable().proxy_for(instance);
}

REGISTER_SERVICE(neutrino.platform.spawn, SpawnService::create);

class close_file_descriptor {
public:
  static void release(int fd) {
    LOG().info("Closed file descriptor %", args(fd));
    close(fd);
  }
};

// A single pipe with two end-points, one that input can be written
// into and one that it can be read from.
class Pipe {
public:
  Pipe() : read_fd_(0), write_fd_(0) { }
  boole open();
  int read_fd() { return read_fd_; }
  int write_fd() { return write_fd_; }
private:
  int read_fd_;
  int write_fd_;
};

// One end of a channel, where input can be read from and sent to the
// other end.
class HalfChannel {
public:
  HalfChannel(int in_fd, int out_fd)
    : in_fd_(in_fd), out_fd_(out_fd) { }
  boole set_remain_open_on_exec();
  int in_fd() const { return in_fd_; }
  int out_fd() const { return out_fd_; }
private:
  static boole set_remain_open_on_exec(int fd);
  int in_fd_;
  int out_fd_;
};

// A duplex communication channel made up of two pipes.
class Channel {
public:
  boole open();
  HalfChannel there() { return HalfChannel(to().read_fd(), from().write_fd()); }
  HalfChannel here() { return HalfChannel(from().read_fd(), to().write_fd()); }
private:
  Pipe &to() { return to_; }
  Pipe &from() { return from_; }
  Pipe to_;
  Pipe from_;
};

class FileSocket : public IStream {
public:
  explicit FileSocket(const HalfChannel &channel)
    : channel_(channel)
    , in_(channel.in_fd())
    , out_(channel.out_fd()) { }
  word write(const vector<uint8_t> &data);
  word read(vector<uint8_t> data);
  void send_message(MiniHeap &heap, p::String name, p::Array args,
      bool is_synchronous);
  boole receive_message(MessageIn &message);
  boole send_reply(MessageIn &message, p::Value value);
  p::Value receive_reply(MiniHeap &heap);
private:
  HalfChannel &channel() { return channel_; }
  int in() { return *in_; }
  int out() { return *out_; }
  HalfChannel channel_;
  own_resource<int, close_file_descriptor> in_;
  own_resource<int, close_file_descriptor> out_;
};

word FileSocket::write(const vector<uint8_t> &data) {
  int bytes = ::write(out(), data.start(), data.length());
  assert bytes == data.length();
  return bytes;
}

struct MessageHeader {
  struct Data {
    uint32_t heap_size;
    uint32_t name;
    uint32_t args;
    uint32_t is_synchronous;
  };
  union {
    Data data;
    uint8_t bytes[sizeof(Data)];
  };
};

struct ReplyHeader {
  struct Data {
    uint32_t heap_size;
    uint32_t value;
  };
  union {
    Data data;
    uint8_t bytes[sizeof(Data)];
  };
};

void FileSocket::send_message(MiniHeap &heap, p::String name,
    p::Array args, bool is_synchronous) {
  MessageHeader header;
  vector<uint8_t> memory = heap.memory();
  header.data.heap_size = memory.length();
  header.data.name = static_cast<uint32_t>(name.data());
  header.data.args = static_cast<uint32_t>(args.data());
  header.data.is_synchronous = is_synchronous;
  write(vector<uint8_t>(header.bytes, sizeof(header.bytes)));
  write(memory);
}

boole FileSocket::receive_message(MessageIn &message) {
  MessageHeader header;
  read(vector<uint8_t>(header.bytes, sizeof(header.bytes)));
  own_vector<uint8_t> memory(vector<uint8_t>::allocate(header.data.heap_size));
  read(memory.as_vector());
  FrozenMiniHeap *heap = new FrozenMiniHeap(memory.release());
  message.set_selector(p::String(header.data.name, &heap->dtable()));
  message.set_args(p::Array(header.data.args, &heap->dtable()));
  message.set_stream(this);
  message.set_is_synchronous(header.data.is_synchronous);
  message.take_ownership(heap);
  return Success::make();
}

boole FileSocket::send_reply(MessageIn &message, p::Value value) {
  if (!message.is_synchronous()) {
    LOG().warn("Reply to asynchronous message ignored.", args());
    return Success::make();
  }
  assert value.impl_id() == MiniHeap::id();
  ReplyHeader header;
  MiniHeap *heap = MiniHeap::get(value.dtable());
  vector<uint8_t> memory;
  if (heap != NULL)
    memory = heap->memory();
  header.data.heap_size = memory.length();
  header.data.value = value.data();
  write(vector<uint8_t>(header.bytes, sizeof(header.bytes)));
  write(memory);
  return Success::make();
}

p::Value FileSocket::receive_reply(MiniHeap &heap) {
  ReplyHeader header;
  read(vector<uint8_t>(header.bytes, sizeof(header.bytes)));
  own_vector<uint8_t> memory(vector<uint8_t>::allocate(header.data.heap_size));
  read(memory.as_vector());
  FrozenMiniHeap *new_heap = new FrozenMiniHeap(memory.release());
  heap.take_ownership(new_heap);
  return p::Value(header.data.value, &new_heap->dtable());
}

word FileSocket::read(vector<uint8_t> data) {
  int bytes = ::read(in(), data.start(), data.length());
  assert bytes == data.length();
  return bytes;
}

class ChildProcess::Data : public p::DTable, public p::Object::Handler {
public:
  Data(pid_t child, const HalfChannel &channel)
    : child_(child)
    , socket_(channel) {
    object = this;
  }
  pid_t child() { return child_; }
  FileSocket &socket() { return socket_; }
  virtual p::Value object_send(p::Object obj, p::String name, p::Array args,
      p::MessageData *data, bool is_synchronous);
private:
  pid_t child_;
  FileSocket socket_;
};

p::Value ChildProcess::Data::object_send(p::Object obj, p::String name,
    p::Array args, p::MessageData *mdata, bool is_synchronous) {
  ChildProcess::Data *data = static_cast<ChildProcess::Data*>(obj.dtable());
  assert name.impl_id() == MiniHeap::id();
  MiniHeap &buffer = *MiniHeap::get(name.dtable());
  FileSocket &socket = data->socket();
  socket.send_message(buffer, name, args, is_synchronous);
  if (is_synchronous) {
    return socket.receive_reply(buffer);
  } else {
    return p::Value();
  }
}

class ParentProcess::Data : public p::DTable, public p::Object::Handler {
public:
  Data(const HalfChannel &channel) : socket_(channel) {
    object = this;
  }
  FileSocket &socket() { return socket_; }
  virtual p::Value object_send(p::Object obj, p::String name,
      p::Array args, p::MessageData *data, bool is_synchronous);
private:
  FileSocket socket_;
};

p::Value ParentProcess::Data::object_send(p::Object obj, p::String name,
    p::Array args, p::MessageData *mdata, bool is_synchronous) {
  ParentProcess::Data *data = static_cast<ParentProcess::Data*>(obj.dtable());
  assert name.impl_id() == MiniHeap::id();
  MiniHeap &buffer = *MiniHeap::get(name.dtable());
  FileSocket &socket = data->socket();
  socket.send_message(buffer, name, args, is_synchronous);
  if (is_synchronous) {
    return socket.receive_reply(buffer);
  } else {
    return p::Value();
  }
}

boole Pipe::open() {
  embed_array<int, 2> pipes;
  if (::pipe(pipes.start()) == -1) {
    LOG().error("Error creating pipe (%)", args(string(::strerror(errno))));
    return InternalError::make(InternalError::ieSystem);
  }
  read_fd_ = pipes[0];
  write_fd_ = pipes[1];
  LOG().info("Opened pipe, read: %, write: %", args(read_fd_, write_fd_));
  return Success::make();
}

boole HalfChannel::set_remain_open_on_exec() {
  try set_remain_open_on_exec(in_fd_);
  return set_remain_open_on_exec(out_fd_);
}

boole HalfChannel::set_remain_open_on_exec(int fd) {
  if (::fcntl(fd, F_SETFD, 0) == -1) {
    LOG().error("Error fcntl-ing pipe (%)", args(string(::strerror(errno))));
    return InternalError::make(InternalError::ieSystem);
  }
  return Success::make();
}

// ---
// C h i l d   P r o c e s s
// ---

ChildProcess::ChildProcess() : data_(NULL) { }

ChildProcess::~ChildProcess() { }

boole Channel::open() {
  try to().open();
  return from().open();
}

boole ChildProcess::open(string &command, vector<string> &args,
    vector< pair<string, string> > &env) {
  // Create a full communication channel.
  Channel channel;
  try channel.open();
  // Make the child's end of the channel stay open after the process
  // has been replaced.
  HalfChannel there = channel.there();
  try there.set_remain_open_on_exec();
  pid_t pid = ::fork();
  if (pid == -1) {
    LOG().error("Error forking (%)", neutrino::args(string(::strerror(errno))));
    return InternalError::make(InternalError::ieSystem);
  } else if (pid == 0) {
    // We've successfully created the child process.  Now run the executable.
    const char *file = command.start();
    // Copy arguments into raw null-terminated array.
    array<char*> arg_arr = TO_ARRAY(char*,
        new char*[args.length() + 1], args.length() + 1);
    for (word i = 0; i < args.length(); i++)
      arg_arr[i] = ::strdup(args[i].start());
    arg_arr[args.length()] = NULL;
    // Copy environmant pairs ("x", "y") into a null-terminated list of
    // strings "x=y" strings.
    array<char*> env_arr = TO_ARRAY(char*,
        new char*[env.length() + 2], env.length() + 2);
    for (word i = 0; i < env.length(); i++) {
      string_stream stream;
      stream.add("%=%", neutrino::args(env[i].first(), env[i].second()));
      env_arr[i] = strdup(stream.raw_c_str().start());
    }
    string_stream stream;
    stream.add("%=%:%", neutrino::args(kMasterEnvVariable, there.in_fd(),
        there.out_fd()));
    env_arr[env.length()] = strdup(stream.raw_c_str().start());
    env_arr[env.length() + 1] = NULL;
    ::execve(file, arg_arr.start(), env_arr.start());
    assert false;
    return Success::make();
  } else {
    // Continue in the parent process.
    data_.set(new ChildProcess::Data(pid, channel.here()));
    return Success::make();
  }
}

p::Object ChildProcess::proxy() {
  return p::Object(0, data());
}

word ChildProcess::wait() {
  while (true) {
    int stat = 0;
    pid_t pid = ::waitpid(data()->child(), &stat, 0);
    if (pid != -1) {
      assert pid == data()->child();
      return stat;
    } else if (errno == EINTR) {
      // If the system call gets interrupted we just try again.  Note
      // that this requires errno to be thread local.
      continue;
    } else {
      LOG().error("Error waiting for child (%)", args(string(::strerror(errno))));
      return -1;
    }
  }
}

boole ChildProcess::receive(MessageIn &message) {
  return data()->socket().receive_message(message);
}

// ---
// P a r e n t   P r o c e s s
// ---

ParentProcess::ParentProcess() : data_(NULL) { }

ParentProcess::~ParentProcess() { }

boole ParentProcess::open() {
  const char *raw_master = getenv(kMasterEnvVariable.start());
  if (raw_master == NULL) {
    LOG().error("Could not read master variable", args(0));
    return InternalError::make(InternalError::ieEnvironment);
  }
  string master = raw_master;
  char *end = NULL;
  int in_fd = strtol(master.start(), &end, 10);
  if (*end != ':') {
    LOG().error("Error parsing master fd %", args(master));
    return InternalError::make(InternalError::ieEnvironment);
  }
  end++;
  int out_fd = strtol(end, &end, 10);
  if (end != master.end()) {
    LOG().error("Error parsing master fd %", args(master));
    return InternalError::make(InternalError::ieEnvironment);
  }
  data_.set(new ParentProcess::Data(HalfChannel(in_fd, out_fd)));
  LOG().info("Opened connection to master through %", args(master));
  return Success::make();
}

boole ParentProcess::receive(MessageIn &message) {
  return data()->socket().receive_message(message);
}

p::Object ParentProcess::proxy() {
  return p::Object(0, data());
}

} // namespace neutrino
