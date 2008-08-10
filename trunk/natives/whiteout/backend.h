#ifndef _NATIVES_WHITEOUT_BACKEND
#define _NATIVES_WHITEOUT_BACKEND

#include <cairo.h>

namespace whiteout {


class RegisterBackend;


class CairoPainter {
public:
  virtual void repaint(cairo_t *cr) = 0;
};


class Frame {
public:
  virtual ~Frame() { }
};


class Backend {
public:
  virtual ~Backend() { }
  virtual Frame *open(int width, int height) = 0;
  static Backend *create(CairoPainter &painter);
  CairoPainter &painter() { return painter_; }
protected:
  Backend(CairoPainter &painter) : painter_(painter) { }
private:
  friend class RegisterBackend;
  static RegisterBackend *first_;
  CairoPainter &painter_;
};


class RegisterBackend {
public:
  typedef Backend *(backend_constructor_t)(CairoPainter &);
  RegisterBackend(backend_constructor_t *cons);
  Backend *create(CairoPainter &painter) { return cons_(painter); }
private:
  backend_constructor_t *cons_;
  RegisterBackend *next_;
};


}

#endif // _NATIVES_WHITEOUT_BACKEND
