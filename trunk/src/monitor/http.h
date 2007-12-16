#ifndef _WEB_WEB
#define _WEB_WEB

namespace neutrino {

class WebServer {
public:
  static WebServer *make();
  
  virtual void start() = 0;
  virtual void stop() = 0;
};

}

#endif // _WEB_WEB
