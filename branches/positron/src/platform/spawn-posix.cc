#include "platform/spawn.h"
#include "utils/check-inl.h"
#include "utils/smart-ptrs-inl.h"
#include "utils/log.h"
#include "utils/array-inl.h"

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

namespace positron {

static const string kMasterEnvVariable = "POSITRON_MASTER";

// A single pipe with two end-points, one that input can be written
// into and one that it can be read from.
class Pipe {
public:
  Pipe() : read_fd_(0), write_fd_(0) { }
  bool open();
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
  bool set_remain_open_on_exec();
  int in_fd() const { return in_fd_; }
  int out_fd() const { return out_fd_; }
private:
  static bool set_remain_open_on_exec(int fd);
  int in_fd_;
  int out_fd_;
};

// A duplex communication channel made up of two pipes.
class Channel {
public:
  bool open();
  HalfChannel there() { return HalfChannel(to().read_fd(), from().write_fd()); }
  HalfChannel here() { return HalfChannel(from().read_fd(), to().write_fd()); }
private:
  Pipe &to() { return to_; }
  Pipe &from() { return from_; }
  Pipe to_;
  Pipe from_;
};

class FileSocket : public ISocket {
public:
  explicit FileSocket(const HalfChannel &channel)
    : channel_(channel)
    , in_(channel.in_fd())
    , out_(channel.out_fd()) { }
  virtual word write(const vector<uint8_t> &data);
  virtual word read(vector<uint8_t> &data);
private:
  HalfChannel &channel() { return channel_; }
  int in() { return in_; }
  int out() { return out_; }
  HalfChannel channel_;
  int in_;
  int out_;
};

word FileSocket::write(const vector<uint8_t> &data) {
  int bytes = ::write(out(), data.start(), data.length());
  assert bytes == data.length();
  return bytes;
}

word FileSocket::read(vector<uint8_t> &data) {
  int bytes = ::read(in(), data.start(), data.length());
  assert bytes == data.length();
  return bytes;
}

class ChildProcess::Data {
public:
  Data(pid_t child, const FileSocket &socket)
    : child_(child)
    , socket_(socket) { }
  pid_t child() { return child_; }
  FileSocket &socket() { return socket_; }
private:
  pid_t child_;
  FileSocket socket_;
};

class ParentProcess::Data {
public:
  Data(const FileSocket &socket) : socket_(socket) { }
  FileSocket &socket() { return socket_; }
private:
  FileSocket socket_;
};

bool Pipe::open() {
  embed_array<int, 2> pipes;
  if (::pipe(pipes.start()) == -1) {
    LOG().error("Error creating pipe (%)", positron::args(string(::strerror(errno))));
    return false;
  }
  read_fd_ = pipes[0];
  write_fd_ = pipes[1];
  return true;
}

bool HalfChannel::set_remain_open_on_exec() {
  return set_remain_open_on_exec(in_fd_)
      && set_remain_open_on_exec(out_fd_);
}

bool HalfChannel::set_remain_open_on_exec(int fd) {
  if (::fcntl(fd, F_SETFD, 0) == -1) {
    LOG().error("Error fcntl-ing pipe (%)", positron::args(string(::strerror(errno))));
    return false;
  }
  return true;
}

bool Channel::open() {
  return to().open() && from().open();
}

bool ChildProcess::open() {
  // Create a full communication channel.
  Channel channel;
  if (!channel.open())
    return false;
  FileSocket own_socket(channel.here());
  // Make the child's end of the channel stay open after the process
  // has been replaced.
  HalfChannel there = channel.there();
  if (!there.set_remain_open_on_exec())
    return false;
  pid_t pid = ::fork();
  if (pid == -1) {
    LOG().error("Error forking (%)", positron::args(string(::strerror(errno))));
    return false;
  } else if (pid == 0) {
    // We've successfully created the child process.  Now run the executable.
    const char *file = command().start();
    // Copy arguments into raw null-terminated array.
    array<char*> arg_arr = TO_ARRAY(char*,
        new char*[args().length() + 1], args().length() + 1);
    for (word i = 0; i < args().length(); i++)
      arg_arr[i] = ::strdup(args()[i].start());
    arg_arr[args().length()] = NULL;
    // Copy environmant pairs ("x", "y") into a null-terminated list of
    // strings "x=y" strings.
    array<char*> env_arr = TO_ARRAY(char*,
        new char*[env().length() + 2], env().length() + 2);
    for (word i = 0; i < env().length(); i++) {
      string_stream stream;
      stream.add("%=%", positron::args(env()[i][0], env()[i][1]));
      env_arr[i] = strdup(stream.raw_c_str().start());
    }
    string_stream stream;
    stream.add("%=%:%", positron::args(kMasterEnvVariable, there.in_fd(),
        there.out_fd()));
    env_arr[env().length()] = strdup(stream.raw_c_str().start());
    env_arr[env().length() + 1] = NULL;
    ::execve(file, arg_arr.start(), env_arr.start());
    assert false;
    return true;
  } else {
    // Continue in the parent process.
    data_ = new ChildProcess::Data(pid, own_socket);
    return true;
  }
}

ISocket &ChildProcess::socket() {
  return data()->socket();
}

word ChildProcess::wait() {
  int stat;
  ::waitpid(data()->child(), &stat, 0);
  return stat;
}

bool ParentProcess::open() {
  const char *raw_master = getenv(kMasterEnvVariable.start());
  if (raw_master == NULL) {
    LOG().error("Could not read master variable", positron::args(0));
    return false;
  }
  string master = raw_master;
  char *end = NULL;
  int in_fd = strtol(master.start(), &end, 10);
  if (*end != ':') {
    LOG().error("Error parsing master fd %", positron::args(master));
    return false;
  }
  end++;
  int out_fd = strtol(end, &end, 10);
  if (end != master.end()) {
    LOG().error("Error parsing master fd %", positron::args(master));
    return false;
  }
  FileSocket socket(HalfChannel(in_fd, out_fd));
  data_ = new ParentProcess::Data(socket);
  LOG().info("Opened connection to master through %", positron::args(master));
  return true;
}

ISocket &ParentProcess::socket() {
  return data()->socket();
}

} // namespace positron
