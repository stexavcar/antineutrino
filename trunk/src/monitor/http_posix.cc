#include "monitor/http.h"
#include "monitor/web.h"
#include "utils/checks.h"
#include "utils/conditions.h"
#include "utils/globals.h"
#include "platform/abort.h"

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

using namespace neutrino;

class PosixWebServer : public WebServer , public Resource {
public:
  PosixWebServer();
  virtual void start();
  virtual void stop();
  virtual void cleanup();
private:
  static void *server_thread_start(void *arg);
  
  /**
   * Sets up and runs the loop accepting connections.  This call
   * blocks until another thread calls stop().
   */
  void run();
  
  /**
   * Opens the server socket listening for http connections.
   */
  bool set_up_listener();
  
  /**
   * Reads a request from a connection and responds.
   */
  void respond(int32_t client);
  
  /**
   * Transmits a reply back to the requester.
   */
  void send_reply(int32_t client, HttpReply &reply);

  bool keep_going() { return keep_going_; }
  pthread_t thread_;
  bool keep_going_;
  int32_t listener_;
};

WebServer *WebServer::make() {
  return new PosixWebServer();
}

PosixWebServer::PosixWebServer()
    : keep_going_(true)
    , listener_(-1) {
  memset(&listener_, 0, sizeof(listener_));
}

void PosixWebServer::start() {
  install();
  pthread_create(&thread_, NULL, &server_thread_start, this);
}

void PosixWebServer::cleanup() {
  if (listener_ >= 0) {
    if (shutdown(listener_, SHUT_RDWR) != 0) {
      fprintf(stderr, "%s\n", strerror(errno));
    }
    if (close(listener_) != 0) {
      fprintf(stderr, "%s\n", strerror(errno));
    }
  }
}

void PosixWebServer::stop() {
  uninstall();
  keep_going_ = false;
  cleanup();
  pthread_join(thread_, NULL);  
}

void *PosixWebServer::server_thread_start(void *arg) {
  PosixWebServer *server = static_cast<PosixWebServer*>(arg);
  server->run();
  return 0;
}

bool PosixWebServer::set_up_listener() {
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(8008);
  listener_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listener_ < 0) {
    Conditions::get().error_occurred(strerror(errno));
    return false;
  }
  if (bind(listener_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    Conditions::get().error_occurred(strerror(errno));
    return false;
  }
  if (listen(listener_, 5) < 0) {
    Conditions::get().error_occurred(strerror(errno));
    return false;
  }
  return true;
}

void PosixWebServer::send_reply(int32_t client, HttpReply &reply) {
  static const uint32_t kSize = 1024;
  char buffer[kSize];
  uint32_t count = snprintf(buffer, kSize, 
    "HTTP/1.1 %i OK\n"
    "Content-Length: %i\n"
    "Connection: close\n\n",
    reply.status(), reply.size());
  send(client, buffer, count, 0);
  send(client, reply.contents(), reply.size(), 0);
}

void PosixWebServer::respond(int32_t client) {
  static const size_t kSize = 1024;
  char request[kSize];
  while (keep_going()) {
    ssize_t bytes_read = recv(client, request, kSize, 0);
    if (bytes_read <= 0) return;
    request[bytes_read] = '\0';
    char filename[kSize];
    if (sscanf(request, "GET %256s HTTP", filename) == 1) {
      HttpReply &reply = FileProvider::get_reply(filename);
      send_reply(client, reply);
      delete &reply;
    }
  }
}

void PosixWebServer::run() {
  if (!set_up_listener()) return;
  while (keep_going()) {
    int32_t client = accept(listener_, NULL, NULL);
    if (client < 0) return;
    respond(client);
    close(client);
  }
}
