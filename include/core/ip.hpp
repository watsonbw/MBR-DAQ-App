#pragma once

#include <string>

// https://en.wikipedia.org/wiki/IPv4
struct IpV4 {
    std::string First;
    std::string Second;
    std::string Third;
    std::string Fourth;
    std::string Port;

    [[nodiscard]] bool        Valid() const;
    [[nodiscard]] bool        AnyEmpty() const;
    [[nodiscard]] std::string String() const;
};

static const IpV4 DEFAULT_IP = {
    .First = "192", .Second = "168", .Third = "4", .Fourth = "1", .Port = "80"};
