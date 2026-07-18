#pragma once

#include <string>

namespace mbr {

// https://en.wikipedia.org/wiki/IPv4
struct ipv4_t {
    std::string first{"192"};
    std::string second{"168"};
    std::string third{"4"};
    std::string fourth{"1"};
    std::string port{"80"};

    [[nodiscard]] bool        is_valid() const;
    [[nodiscard]] bool        any_empty() const;
    [[nodiscard]] std::string to_string() const;
};

} // namespace mbr
