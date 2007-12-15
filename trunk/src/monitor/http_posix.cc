#include "monitor/http.h"
#include "utils/conditions.h"
#include "utils/globals.h"

#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>

using namespace neutrino;

void WebServer::start() {
  pthread_t thread;
  pthread_create(&thread, NULL, &run_server, NULL);
  pthread_join(thread, NULL);
}

void *WebServer::run_server(void *) {
  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(8080);
  
  int32_t listener = socket(AF_INET, SOCK_STREAM, 0);
  if (listener < 0) {
    Conditions::get().error_occurred("Could not open server socket");
    return 0;
  }
  if (bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    Conditions::get().error_occurred("Could not bind server socket");
    return 0;
  }
  if (listen(listener, 5) < 0) {
    Conditions::get().error_occurred("Call to listen failed");
    return 0;
  }

  while (true) {
    int32_t connection = accept(listener, NULL, NULL);
    if (connection < 0) {
      Conditions::get().error_occurred("Error accepting connection");
      return 0;
    }
  }
}
