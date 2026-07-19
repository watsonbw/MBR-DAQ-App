#include <catch_amalgamated.hpp>

#include "core/ip.hpp"

namespace mbr::tests {

TEST_CASE("ipv4_t default constructor sets correct local address") {
    ipv4_t ip;
    CHECK(ip.first == "192");
    CHECK(ip.second == "168");
    CHECK(ip.third == "4");
    CHECK(ip.fourth == "1");
    CHECK(ip.port == "80");
    CHECK(ip.is_valid());
    CHECK_FALSE(ip.any_empty());
    CHECK(ip.to_string() == "192.168.4.1:80");
}

TEST_CASE("ipv4_t is_valid check") {
    ipv4_t ip;

    SECTION("Valid boundary values") {
        ip = {"0", "0", "0", "0", "1"};
        CHECK(ip.is_valid());
        ip = {"255", "255", "255", "255", "65535"};
        CHECK(ip.is_valid());
    }

    SECTION("Invalid octet values") {
        ip = {"256", "168", "4", "1", "80"};
        CHECK_FALSE(ip.is_valid());
        ip = {"192", "-1", "4", "1", "80"};
        CHECK_FALSE(ip.is_valid());
    }

    SECTION("Invalid port values") {
        ip = {"192", "168", "4", "1", "0"};
        CHECK_FALSE(ip.is_valid());
        ip = {"192", "168", "4", "1", "65536"};
        CHECK_FALSE(ip.is_valid());
    }

    SECTION("Non-numeric inputs") {
        ip = {"abc", "168", "4", "1", "80"};
        CHECK_FALSE(ip.is_valid());
        ip = {"192", "168", "4", "1", "xyz"};
        CHECK_FALSE(ip.is_valid());
    }
}

TEST_CASE("ipv4_t any_empty check") {
    ipv4_t ip;
    CHECK_FALSE(ip.any_empty());

    ip.first = "";
    CHECK(ip.any_empty());
    CHECK_FALSE(ip.is_valid());

    ip = {"192", "168", "4", "1", ""};
    CHECK(ip.any_empty());
    CHECK_FALSE(ip.is_valid());
}

} // namespace mbr::tests
