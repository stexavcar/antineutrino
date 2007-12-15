#ifndef _WEB_WEB
#define _WEB_WEB

namespace neutrino {

class WebServer {
public:
  static void start();
private:
  static void *run_server(void *);
};

}

#endif // _WEB_WEB
