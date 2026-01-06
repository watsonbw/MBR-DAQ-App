#pragma once

#include <string>

// https://en.wikipedia.org/wiki/IPv4
struct IpV4 {
    std::string First;
    std::string Second;
    std::string Third;
    std::string Fourth;
    std::string Port;

    bool        Valid() const;
    bool        AnyEmpty() const;
    std::string String() const;
};

static IpV4 DEFAULT_IP = {"192", "168", "4", "1", "80"};
