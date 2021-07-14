#include "cppnetlib/base/buffer.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace cppnetlib;
using cppnetlib::net::Buffer;
using std::string;

TEST_CASE("TEST Buffer Append and Retrieve") {
  Buffer buf;
  REQUIRE(buf.readable_bytes() == 0);
  REQUIRE(buf.writeable_bytes() == Buffer::kInitSize);
  REQUIRE(buf.capacity() == Buffer::kInitSize+Buffer::kPrepend);
  REQUIRE(buf.capacity() == 2048);
  REQUIRE(buf.r_pos()-buf.w_pos() == 0);

  const string str(200, 'x');
  buf.Append(str);
  REQUIRE(buf.readable_bytes() == str.size());
  REQUIRE(buf.writeable_bytes() == Buffer::kInitSize - str.size());

  const string str2 = buf.RetrieveAsStr(50);
  REQUIRE(str2.size() == 50);
  REQUIRE(str2 == string(50, 'x'));
  REQUIRE(buf.readable_bytes() == str.size() - str2.size());
  REQUIRE(buf.writeable_bytes() == Buffer::kInitSize - str.size());

  buf.Append(str);
  REQUIRE(buf.readable_bytes() == 2*str.size() - str2.size());
  REQUIRE(buf.writeable_bytes() == Buffer::kInitSize - 2*str.size());

  const string str3 = buf.RetrieveAllAsStr();
  REQUIRE(str3.size() == 350);
  REQUIRE(str3 == string(350, 'x'));
  REQUIRE(buf.readable_bytes() == 0);
  REQUIRE(buf.writeable_bytes() == Buffer::kInitSize);
  REQUIRE(buf.r_pos()-buf.w_pos() == 0);
}

TEST_CASE("Test Buffer Grow") {
  Buffer buf;
  buf.Append(string(400, 'y'));
  REQUIRE(buf.readable_bytes() == 400);
  REQUIRE(buf.writeable_bytes() == Buffer::kInitSize - 400);

  buf.Append(string(2000, 'x'));
  REQUIRE(buf.capacity() == 4096);
  REQUIRE(buf.readable_bytes() == 2400);
  REQUIRE(buf.writeable_bytes() == 0);

  buf.Append(string(1000, 'z'));
  REQUIRE(buf.capacity() == 4096);
  REQUIRE(buf.readable_bytes() == 3400);
  REQUIRE(buf.writeable_bytes() == 0);

  buf.Retrieve(2000);
  REQUIRE(buf.capacity() == 4096);
  REQUIRE(buf.readable_bytes() == 1400);
  REQUIRE(buf.writeable_bytes() == 0);

  buf.Append(string(800, 'm'));
  REQUIRE(buf.capacity() == 4096);
  REQUIRE(buf.readable_bytes() == 2200);
  REQUIRE(buf.writeable_bytes() == 1200);
}

TEST_CASE("Test Buffer Grow Again") {
  Buffer buf;
  buf.Append(string(2050, 'x'));
  REQUIRE(buf.capacity() == 4096);
  REQUIRE(buf.readable_bytes() == 2050);
  REQUIRE(buf.writeable_bytes() == 0);

  buf.Retrieve(1500);
  REQUIRE(buf.capacity() == 4096);
  REQUIRE(buf.readable_bytes() == 550);
  REQUIRE(buf.writeable_bytes() == 0);

  buf.Append(string(2050, 'y'));
  REQUIRE(buf.capacity() == 4096);
  REQUIRE(buf.readable_bytes() == 2600);
  REQUIRE(buf.writeable_bytes() == 0);
}