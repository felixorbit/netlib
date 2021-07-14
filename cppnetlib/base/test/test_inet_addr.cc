#include "cppnetlib/base/inet_addr.h"

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

using cppnetlib::net::InetAddr;
using std::string;


TEST_CASE("Test InetAddr") {
  InetAddr addr0(1234);
  REQUIRE(addr0.GetIPStr() == string("0.0.0.0"));
  REQUIRE(addr0.GetIPPortStr() == string("0.0.0.0:1234"));
  REQUIRE(addr0.GetPort() == 1234);

  InetAddr addr1(4321, true);
  REQUIRE(addr1.GetIPStr() == string("127.0.0.1"));
  REQUIRE(addr1.GetIPPortStr() == string("127.0.0.1:4321"));
  REQUIRE(addr1.GetPort() == 4321);

  InetAddr addr2("1.2.3.4", 8888);
  REQUIRE(addr2.GetIPStr() == string("1.2.3.4"));
  REQUIRE(addr2.GetIPPortStr() == string("1.2.3.4:8888"));
  REQUIRE(addr2.GetPort() == 8888);

  InetAddr addr3("255.254.253.252", 65535);
  REQUIRE(addr3.GetIPStr() == string("255.254.253.252"));
  REQUIRE(addr3.GetIPPortStr() == string("255.254.253.252:65535"));
  REQUIRE(addr3.GetPort() == 65535);
}