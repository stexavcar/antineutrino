#ifndef _MONITOR_WEB
#define _MONITOR_WEB

#include "utils/globals.h"
#include "utils/list.h"
#include "utils/string.h"

namespace neutrino {

class HttpReply {
public:
  HttpReply(uint32_t status, const byte *contents, uint32_t size,
      bool owns_contents);
  HttpReply(string str);
  ~HttpReply();
  uint32_t status() { return status_; }
  void override_status(uint32_t value) { status_ = value; }
  const byte *contents() { return contents_; }
  uint32_t size() { return size_; }
private:
  uint32_t status_;
  const byte *contents_;
  uint32_t size_;
  bool owns_contents_;
};

class FileProvider {
public:
  static HttpReply &get_reply(string name);
private:
  
  /**
   * Returns the contents of the file with the specified path.  Takes
   * care of disposing the path.
   */
  static HttpReply *get_reply(list_buffer<string> &path);

  /**
   * Divides a path into segments and adds them to the end of the
   * given list buffer.
   */
  static void explode_path(string path, list_buffer<string> &parts);
  
  /**
   * Returns a static resource with the specified name.
   */
  static HttpReply *get_resource(list_buffer<string> &path);

};

}

#endif // _MONITOR_WEB
