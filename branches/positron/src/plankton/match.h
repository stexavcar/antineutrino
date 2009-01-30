#ifndef _PLANKTON_MATCH
#define _PLANKTON_MATCH

namespace neutrino {
namespace plankton {

class Pattern {
public:
  virtual bool match_array(p::Array that) { return false; }
  virtual bool match_string(p::String that) { return false; }
  virtual bool match_integer(p::Integer that) { return false; }
  virtual bool match_null(p::Null that) { return false; }
  virtual bool match_object(p::Object that) { return false; }
};

} // plankton
} // neutrino

#endif // _PLANKTON_MATCH
