#include <limits>

#include "cppnetlib/core/log_stream.h"

// catch2 provide a main function
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using namespace cppnetlib;
using std::string;

TEST_CASE("Test LogStream Boolean") {
  LogStream os;
  const LogStream::Buffer& buf = os.buffer();

  REQUIRE(buf.ToString() == string(""));
  os << true;
  REQUIRE(buf.ToString() == string("1"));
  os << '\n';
  REQUIRE(buf.ToString() == string("1\n"));
  os << false;
  REQUIRE(buf.ToString() == string("1\n0"));
}

TEST_CASE("Test LogStream Integers") {
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    REQUIRE(buf.ToString() == string(""));
    
    os << 1;
    REQUIRE(buf.ToString() == string("1"));
    os << 0;
    REQUIRE(buf.ToString() == string("10"));
    os << -1;
    REQUIRE(buf.ToString() == string("10-1"));
    
    os.ResetBuffer();

    os << 0 << " " << 123 << 'x' << 0x64;
    REQUIRE(buf.ToString() == string("0 123x100"));
}

TEST_CASE("Test LogStream Integers Limits") {
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << -2147483647;
    REQUIRE(buf.ToString() == string("-2147483647"));
    os << static_cast<int>(-2147483647 - 1);
    REQUIRE(buf.ToString() == string("-2147483647-2147483648"));
    os << ' ';
    os << 2147483647;
    REQUIRE(buf.ToString() == string("-2147483647-2147483648 2147483647"));
    
    os.ResetBuffer();

    os << std::numeric_limits<int16_t>::min();
    REQUIRE(buf.ToString() == string("-32768"));
    os.ResetBuffer();

    os << std::numeric_limits<int16_t>::max();
    REQUIRE(buf.ToString() == string("32767"));
    os.ResetBuffer();

    os << std::numeric_limits<uint16_t>::min();
    REQUIRE(buf.ToString() == string("0"));
    os.ResetBuffer();

    os << std::numeric_limits<uint16_t>::max();
    REQUIRE(buf.ToString() == string("65535"));
    os.ResetBuffer();

    os << std::numeric_limits<int32_t>::min();
    REQUIRE(buf.ToString() == string("-2147483648"));
    os.ResetBuffer();

    os << std::numeric_limits<int32_t>::max();
    REQUIRE(buf.ToString() == string("2147483647"));
    os.ResetBuffer();

    os << std::numeric_limits<uint32_t>::min();
    REQUIRE(buf.ToString() == string("0"));
    os.ResetBuffer();

    os << std::numeric_limits<uint32_t>::max();
    REQUIRE(buf.ToString() == string("4294967295"));
    os.ResetBuffer();

    os << std::numeric_limits<int64_t>::min();
    REQUIRE(buf.ToString() == string("-9223372036854775808"));
    os.ResetBuffer();

    os << std::numeric_limits<int64_t>::max();
    REQUIRE(buf.ToString() == string("9223372036854775807"));
    os.ResetBuffer();

    os << std::numeric_limits<uint64_t>::min();
    REQUIRE(buf.ToString() == string("0"));
    os.ResetBuffer();

    os << std::numeric_limits<uint64_t>::max();
    REQUIRE(buf.ToString() == string("18446744073709551615"));
    os.ResetBuffer();

    int16_t a = 0;
    int32_t b = 0;
    int64_t c = 0;
    os << a;
    os << b;
    os << c;
    REQUIRE(buf.ToString() == string("000"));
}

TEST_CASE("Test LogStream Floats") {
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << 0.0;
    REQUIRE(buf.ToString() == string("0"));
    os.ResetBuffer();

    os << 1.0;
    REQUIRE(buf.ToString() == string("1"));
    os.ResetBuffer();

    double a = 0.1;
    double b = 0.05;
    os << a+b;
    REQUIRE(buf.ToString() == string("0.15"));
    os.ResetBuffer();

    os << -123.456;
    REQUIRE(buf.ToString() == string("-123.456"));
    os.ResetBuffer();

    os << a+b << -123.0 << " " << -123.456;
    REQUIRE(buf.ToString() == string("0.15-123 -123.456"));
}

TEST_CASE("Test LogStream Void Ptr") {
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << static_cast<void*>(0);
    REQUIRE(buf.ToString() == string("0x0"));
    os.ResetBuffer();

    os << reinterpret_cast<const void*>(8888);
    REQUIRE(buf.ToString() == string("0x22B8"));
    os.ResetBuffer();

    os << reinterpret_cast<void*>(8888);
    REQUIRE(buf.ToString() == string("0x22B8"));
    os.ResetBuffer();
}

TEST_CASE("Test LogStream Strings") {
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << "Hello ";
    REQUIRE(buf.ToString() == string("Hello "));

    string name = "Cpp Net LIB";
    os << name;
    REQUIRE(buf.ToString() == string("Hello Cpp Net LIB"));
}

TEST_CASE("Test LogStream Fmt") {
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << fmt::format("{:4d}", 1);
    REQUIRE(buf.ToString() == string("   1"));
    os.ResetBuffer();

    os << fmt::format("{:4.2f}", 1.2);
    REQUIRE(buf.ToString() == string("1.20"));
    os.ResetBuffer();

    os << fmt::format("{:4.2f}", 1.2) << fmt::format("{:4d}", 43);
    REQUIRE(buf.ToString() == string("1.20  43"));
    os.ResetBuffer();
}

TEST_CASE("Test LogStream Long String Stream") {
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    // write 3990 bytes to buffer
    for (int i = 0; i < 399; i++) {
        os << "123456789 ";
        REQUIRE(buf.length() == 10*(i+1));
        REQUIRE(buf.available() == 4000 - 10*(i+1));
    }

    // this time, these 10 bytes will be dropped
    os << "abcdefghi ";
    REQUIRE(buf.length() == 3990);
    REQUIRE(buf.available() == 10);

    // try to write 9 bytes
    os << "abcdefghi";
    REQUIRE(buf.length() == 3999);
    REQUIRE(buf.available() == 1);
}

