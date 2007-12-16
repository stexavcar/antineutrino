#include "monitor/web.h"
#include "utils/list-inl.h"
#include "utils/string-inl.h"

#include <string.h>

namespace neutrino {

HttpReply::HttpReply(uint32_t status, const byte *contents, uint32_t size)
    : status_(status), contents_(contents), size_(size) { }

HttpReply &FileProvider::get_reply(string name) {
  // Try to get a reply for the requested file
  list_buffer<string> path;
  explode_path(name, path);
  HttpReply *reply = FileProvider::get_reply(path);
  if (reply != NULL) return *reply;
  // If no reply could be found return 404
  list_buffer<string> four_o_four;
  explode_path(string::dup("404.html"), four_o_four);
  reply = get_reply(four_o_four);
  ASSERT(reply != NULL);
  reply->override_status(404);
  return *reply;
}

HttpReply *FileProvider::get_reply(list_buffer<string> &path) {
  HttpReply *reply = get_resource(path);
  for (uint32_t i = 0; i < path.length(); i++)
    path[i].dispose();
  return reply;
}

void FileProvider::explode_path(string name, list_buffer<string> &path) {
  string_buffer buf;
  for (uint32_t i = 0; i < name.length(); i++) {
    if (name[i] == '/') {
      if (!buf.is_empty()) {
        path.append(buf.to_string());
        buf.clear();
      }
    } else {
      buf.append(name[i]);
    }
  }
  if (buf.is_empty()) path.append(string::dup("index.html"));
  else path.append(buf.to_string());  
}

} // neutrino