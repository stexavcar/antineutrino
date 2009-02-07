#include "plankton/codec-inl.h"
#include "plankton/plankton-inl.h"
#include "test-inl.h"

using namespace neutrino;

static const char *kPlainText =
  "<Ben174> : If they only realized 90% of the overtime they pay me is"
  " only cause i like staying here playing with Kazaa when the bandwidth"
  " picks up after hours."
  "<ChrisLMB> : If any of my employees did that they'd be fired instantly."
  "<Ben174> : Where u work?"
  "<ChrisLMB> : I'm the CTO at LowerMyBills.com"
  "*** Ben174 (BenWright@TeraPro33-41.LowerMyBills.com) Quit (Leaving)";

static const char *kBase16Encoded =
  "PEJlbjE3ND4gOiBJZiB0aGV5IG9ubHkgcmVhbGl6ZWQgOTAlIG9mIHRoZSBvdmVyd"
  "GltZSB0aGV5IHBheSBtZSBpcyBvbmx5IGNhdXNlIGkgbGlrZSBzdGF5aW5nIGhlcm"
  "UgcGxheWluZyB3aXRoIEthemFhIHdoZW4gdGhlIGJhbmR3aWR0aCBwaWNrcyB1cCB"
  "hZnRlciBob3Vycy48Q2hyaXNMTUI+IDogSWYgYW55IG9mIG15IGVtcGxveWVlcyBk"
  "aWQgdGhhdCB0aGV5J2QgYmUgZmlyZWQgaW5zdGFudGx5LjxCZW4xNzQ+IDogV2hlc"
  "mUgdSB3b3JrPzxDaHJpc0xNQj4gOiBJJ20gdGhlIENUTyBhdCBMb3dlck15QmlsbH"
  "MuY29tKioqIEJlbjE3NCAoQmVuV3JpZ2h0QFRlcmFQcm8zMy00MS5Mb3dlck15Qml"
  "sbHMuY29tKSBRdWl0IChMZWF2aW5nKQ==";

static void test_base64(string plain, string encoded) {
  {
    string_stream stream;
    Base64Encoder<string_stream> encoder(stream);
    for (word i = 0; i < plain.length(); i++)
      encoder.add(plain[i]);
    encoder.flush();
    assert stream.raw_c_str() == string(encoded);
  }
  {
    string_stream stream;
    Base64Decoder<string_stream> decoder(stream);
    for (word i = 0; i < encoded.length(); i++)
      decoder.add(encoded[i]);
    assert decoder.flush();
    assert stream.raw_c_str() == string(plain);
  }
  {
    string_stream stream;
    Base64Decoder<string_stream> decoder(stream);
    Base64Encoder< Base64Decoder<string_stream> > encoder(decoder);
    for (word i = 0; i < plain.length(); i++)
      encoder.add(plain[i]);
    encoder.flush();
    assert stream.raw_c_str() == string(plain);
  }
}

TEST(base64) {
  test_base64(kPlainText, kBase16Encoded);
  test_base64("leasure.", "bGVhc3VyZS4=");
  test_base64("easure.", "ZWFzdXJlLg==");
  test_base64("asure.", "YXN1cmUu");
  test_base64("sure.", "c3VyZS4=");
  test_base64("ure.", "dXJlLg==");
  test_base64("re.", "cmUu");
  test_base64("e.", "ZS4=");
  test_base64(".", "Lg==");
  test_base64("", "");
}

class StringReader {
public:
  StringReader(string str) : str_(str), cursor_(0) { }
  uint8_t get() { return str_[cursor_++]; }
private:
  string str_;
  word cursor_;
};

static void test_serialization(p::Value value, string expected) {
  string_stream stream;
  {
    Serializer<string_stream> serializer(stream);
    serializer.serialize(value);
  }
  StringReader reader(stream.raw_c_str());
  Deserializer<StringReader> deserializer(reader);
  p::Value result = deserializer.deserialize();
  string_stream str;
  variant var = value;
  str.add("%", vargs(result));
  assert str.raw_c_str() == expected;
}

TEST(serialize) {
  test_serialization("foobar", "\"foobar\"");
  test_serialization(934, "934");
  test_serialization(p::Array::of(1, 2, 3), "(1 2 3)");
  test_serialization(p::Array::of("a", 0xb, "c"), "(\"a\" 11 \"c\")");
//  test_serialization(p::Null::get(), "(null)");
}
