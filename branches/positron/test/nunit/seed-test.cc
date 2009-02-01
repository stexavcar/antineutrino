#include "io/miniheap-inl.h"
#include "plankton/plankton-inl.h"
#include "test-inl.h"


using namespace neutrino;


class TestSeed {
public:
  TestSeed(p::String species) : species_(species) { }
  p::String species() { return species_; }
  hash_map<p::String, p::Value> &attributes() { return attributes_; }
private:
  hash_map<p::String, p::Value> attributes_;
  p::String species_;
  void *instance_;
};


class SeedFactory : public p::DTable, p::Seed::Handler {
public:
  SeedFactory();
  p::Seed new_seed(p::String species);
  static void set_attribute(p::Seed seed, p::String key, p::Value value);
  virtual p::String seed_species(p::Seed that);
  virtual p::Value seed_get_attribute(p::Seed that, p::String key);
  virtual bool seed_for_each_attribute(p::Seed that, p::Seed::attribute_callback_t iter, void *data);
  virtual void *seed_grow(p::Seed that, p::String species);
private:
  buffer<TestSeed*> &seeds() { return seeds_; }
  own_buffer<TestSeed> seeds_;
};


class Singleton : public p::IClass {
public:
  static p::String species() { return "nunit.Singleton"; }
  static Singleton *construct(p::Seed seed) { return new Singleton(); }
};
REGISTER_CLASS(Singleton);


class Point : public p::IClass {
public:
  Point(p::Seed seed) : seed_(seed) { }
  word x() { return cast<p::Integer>(seed()["x"]).value(); }
  word y() { return cast<p::Integer>(seed()["y"]).value(); }
  static p::String species() { return "nunit.Point"; }
  static Point *construct(p::Seed seed) { return new Point(seed); }
protected:
  p::Seed seed() { return seed_; }
private:
  p::Seed seed_;
};
REGISTER_CLASS(Point);


class Point3D : public Point {
public:
  Point3D(p::Seed seed) : Point(seed) { }
  word z() { return cast<p::Integer>(seed()["z"]).value(); }
  static p::String species() { return "nunit.Point3D"; }
  static Point3D *construct(p::Seed seed) { return new Point3D(seed); }
  static p::String super_class() { return Point::species(); }
};
REGISTER_CLASS(Point3D);


SeedFactory::SeedFactory() {
  seed = this;
}


p::Seed SeedFactory::new_seed(p::String species) {
  TestSeed *seed = new TestSeed(species);
  seeds().append(seed);
  return p::Seed(reinterpret_cast<word>(seed), this);
}


p::String SeedFactory::seed_species(p::Seed that) {
  return reinterpret_cast<TestSeed*>(that.data())->species();
}


p::Value SeedFactory::seed_get_attribute(p::Seed that, p::String key) {
  TestSeed *seed = reinterpret_cast<TestSeed*>(that.data());
  return seed->attributes().get(key, Factory::get_null());
}


void SeedFactory::set_attribute(p::Seed that, p::String key, p::Value value) {
  TestSeed *seed = reinterpret_cast<TestSeed*>(that.data());
  seed->attributes().put(key, value);
}

class SeedAttributeIterator {
public:
  SeedAttributeIterator(p::Seed::attribute_callback_t iter, void *data)
    : iter_(iter), data_(data) { }
  bool operator()(p::String key, p::Value value) const {
    return (iter())(key, value, data());
  }
private:
  p::Seed::attribute_callback_t iter() const { return iter_; }
  void *data() const { return data_; }
  p::Seed::attribute_callback_t iter_;
  void *data_;
};

bool SeedFactory::seed_for_each_attribute(p::Seed that,
    p::Seed::attribute_callback_t iter, void *data) {
  TestSeed *seed = reinterpret_cast<TestSeed*>(that.data());
  return seed->attributes().for_each(SeedAttributeIterator(iter, data));
}


void *SeedFactory::seed_grow(p::Seed that, p::String species) {
  if (!p::Seed::belongs_to(that, species)) return NULL;
  p::IClassRegistryEntry *klass = p::IClassRegistryEntry::lookup(species);
  if (klass == NULL) return NULL;
  return klass->new_instance(that);
}


TEST(simple_seed) {
  SeedFactory factory;
  p::Seed seed = factory.new_seed("nunit.Singleton");
  assert seed.species() == "nunit.Singleton";
  assert is<p::Null>(seed["knort"]);
  assert (is_seed<Singleton>(seed));
  Singleton *singleton = seed.grow<Singleton>();
  assert singleton != static_cast<void*>(NULL);
  Point *point = seed.grow<Point>();
  assert point == static_cast<void*>(NULL);
}


TEST(seed_field_access) {
  SeedFactory factory;
  p::Seed seed = factory.new_seed("nunit.Point");
  SeedFactory::set_attribute(seed, "x", 19);
  SeedFactory::set_attribute(seed, "y", 20);
  assert (is_seed<Point>(seed));
  Point *point = seed.grow<Point>();
  assert point->x() == 19;
  assert point->y() == 20;
}


class VisitInfo {
public:
  VisitInfo() : seen_x(false), seen_y(false), seen_z(false), count(0) { }
  bool seen_x, seen_y, seen_z;
  word count;
};


static bool visit_attribute(p::String key, p::Value value, void *data) {
  VisitInfo &info = *static_cast<VisitInfo*>(data);
  info.count++;
  if (key == "x") {
    info.seen_x = true;
    assert cast<p::Integer>(value).value() == 76;
  }
  if (key == "y") {
    info.seen_y = true;
    assert cast<p::Integer>(value).value() == 45;
  }
  if (key == "z") {
    info.seen_z = true;
    assert cast<p::Integer>(value).value() == 99;
  }
  return true;
}


TEST(subclassing) {
  SeedFactory factory;
  p::Seed seed = factory.new_seed("nunit.Point3D");
  SeedFactory::set_attribute(seed, "x", 76);
  SeedFactory::set_attribute(seed, "y", 45);
  SeedFactory::set_attribute(seed, "z", 99);
  VisitInfo info;
  seed.for_each_attribute(visit_attribute, &info);
  assert info.count == 3;
  assert info.seen_x;
  assert info.seen_y;
  assert info.seen_z;
  assert (is_seed<Point>(seed));
  Point *point = seed.grow<Point>();
  assert point->x() == 76;
  assert point->y() == 45;
  assert static_cast<Point3D*>(point)->z() == 99;
}
