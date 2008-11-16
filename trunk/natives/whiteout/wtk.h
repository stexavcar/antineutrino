#ifndef _WHITEOUT_WTK
#define _WHITEOUT_WTK

#include <cairo.h>

#undef check
#include "utils/list-inl.h"

namespace wtk {

class Quant {
public:
  enum Type { qEmpty, qAbsolute, qScale };
  inline Quant() : type_(qEmpty), quantity_(0) { }
  inline Quant(int value) : type_(qAbsolute), quantity_(value) { }
  inline Quant(double value) : type_(qScale), quantity_(kScaleFactor * value) { }
  int fix(int scale) {
    switch (type_) {
      case qAbsolute:
        return quantity_;
      case qScale:
        return (quantity_ * scale) / kScaleFactor;
      default:
        return 0;
    }
  }
  static Quant scale(const Quant &from, const Quant &to, float t);
private:
  Quant(Type type, int quantity) : type_(type), quantity_(quantity) { }
  static const int kScaleFactor = 1 << 12;
  Type type_ : 16;
  int quantity_ : 16;
};

class Point {
public:
  Point(const Quant &x, const Quant &y)
    : x_(x), y_(y) { }
  Quant &x() { return x_; }
  Quant &y() { return y_; }
private:
  Quant x_;
  Quant y_;
};

class Circle;
class Rect;

class ElementVisitor {
public:
  virtual void visit_circle(Circle &that) = 0;
  virtual void visit_rect(Rect &that) = 0;
};

class Element {
public:
  virtual void accept(ElementVisitor &visitor) = 0;
};

class Circle : public Element {
public:
  Circle(const Point &center, const Quant &radius)
    : center_(center), radius_(radius) { }
  virtual void accept(ElementVisitor &visitor) { visitor.visit_circle(*this); }
  Point &center() { return center_; }
  Quant &radius() { return radius_; }
private:
  Point center_;
  Quant radius_;
};

class Rect : public Element {
public:
  Rect(const Point &top_left, const Quant &width, const Quant &height)
    : top_left_(top_left)
    , width_(width)
    , height_(height)
    , rx_(0)
    , ry_(0) { }
  virtual void accept(ElementVisitor &visitor) { visitor.visit_rect(*this); }
  Point &top_left() { return top_left_; }
  Quant &width() { return width_; }
  Quant &height() { return height_; }
private:
  Point top_left_;
  Quant width_;
  Quant height_;
  Quant rx_;
  Quant ry_;
};

class Animator {
public:
  class Entry {
  public:
    Entry() { }
    Entry(int limit, const Quant &low, const Quant &high,
        Quant *subject)
      : current_(0)
      , limit_(limit)
      , delta_(1)
      , low_(low)
      , high_(high)
      , subject_(subject) { }
  private:
    friend class Animator;
    int current_;
    int limit_;
    int delta_;
    Quant low_;
    Quant high_;
    Quant *subject_;
  };
  void tick();
  void add(int limit, const Quant &low, const Quant &high,
      Quant *subject) {
    entries().append(Entry(limit, low, high, subject));
  }
private:
  neutrino::list_buffer<Entry> &entries() { return entries_; }
  neutrino::list_buffer<Entry> entries_;
};

class Graphics {
public:
  Graphics(Element& root) : root_(root) { }
  Element &root() { return root_; }
  Animator &animator() { return animator_; }
private:
  Element &root_;
  Animator animator_;
};

} // namespace wtk

#endif // _WHITEOUT_WTK
