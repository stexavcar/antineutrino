#include "io/miniheap-inl.h"
#include "plankton/plankton-inl.h"
#include "test-inl.h"

using namespace neutrino;

class TestSeed {
public:
  TestSeed(p::String oid) : oid_(oid) { }
  p::String oid() { return oid_; }
  hash_map<p::String, p::Value> &attributes() { return attributes_; }
private:
  hash_map<p::String, p::Value> attributes_;
  p::String oid_;
  void *instance_;
};

class SeedFactory : public p::Value::DTable {
public:
  SeedFactory();
  p::Seed new_seed(p::String oid);
private:
  static p::String seed_oid(p::Seed that);
  static p::Value seed_get(p::Seed that, p::String key);
  static bool seed_set(p::Seed that, p::String key, p::Value value);
  static void *seed_grow(p::Seed that, p::String oid);
  buffer<TestSeed*> &seeds() { return seeds_; }
  own_buffer<TestSeed> seeds_;
};

class Singleton : public p::IClass {
public:
  static p::String oid() { return "nunit.Singleton"; }
  static Singleton *construct(p::Seed seed) { return new Singleton(); }
};
REGISTER_CLASS(Singleton);

class Point : public p::IClass {
public:
  Point(p::Seed seed) : seed_(seed) { }
  word x() { return cast<p::Integer>(seed()["x"]).value(); }
  word y() { return cast<p::Integer>(seed()["y"]).value(); }
  static p::String oid() { return "nunit.Point"; }
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
  static p::String oid() { return "nunit.Point3D"; }
  static Point3D *construct(p::Seed seed) { return new Point3D(seed); }
  static p::String super_class() { return Point::oid(); }
};
REGISTER_CLASS(Point3D);

SeedFactory::SeedFactory() {
  seed.oid = seed_oid;
  seed.get = seed_get;
  seed.set = seed_set;
  seed.grow = seed_grow;
}

p::Seed SeedFactory::new_seed(p::String oid) {
  TestSeed *seed = new TestSeed(oid);
  seeds().append(seed);
  return p::Seed(reinterpret_cast<word>(seed), this);
}

p::String SeedFactory::seed_oid(p::Seed that) {
  return reinterpret_cast<TestSeed*>(that.data())->oid();
}

p::Value SeedFactory::seed_get(p::Seed that, p::String key) {
  TestSeed *seed = reinterpret_cast<TestSeed*>(that.data());
  return seed->attributes().get(key, Factory::get_null());
}

bool SeedFactory::seed_set(p::Seed that, p::String key, p::Value value) {
  TestSeed *seed = reinterpret_cast<TestSeed*>(that.data());
  seed->attributes().put(key, value);
  return true;
}

void *SeedFactory::seed_grow(p::Seed that, p::String oid) {
  if (!p::Seed::belongs_to(that, oid)) return NULL;
  p::IClassRegistryEntry *klass = p::IClassRegistryEntry::lookup(oid);
  if (klass == NULL) return NULL;
  return klass->new_instance(that);
}

TEST(simple_seed) {
  SeedFactory factory;
  p::Seed seed = factory.new_seed("nunit.Singleton");
  assert seed.oid() == "nunit.Singleton";
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
  seed.set("x", 19);
  seed.set("y", 20);
  assert (is_seed<Point>(seed));
  Point *point = seed.grow<Point>();
  assert point->x() == 19;
  assert point->y() == 20;
}

TEST(subclassing) {
  SeedFactory factory;
  p::Seed seed = factory.new_seed("nunit.Point3D");
  seed.set("x", 76);
  seed.set("y", 45);
  seed.set("z", 99);
  assert (is_seed<Point>(seed));
  Point *point = seed.grow<Point>();
  assert point->x() == 76;
  assert point->y() == 45;
  assert static_cast<Point3D*>(point)->z() == 99;
}
