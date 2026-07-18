#include <algorithm>
#include <cstdint>
#include <optional>
#include <string_view>

#include <fmt/format.h>

#include "core/ip.hpp"

namespace mbr {

bool ipv4_t::is_valid() const {
    const auto validate_digit = [](std::string_view sv) -> std::optional<uint64_t> {
        if (sv.empty()) { return std::nullopt; }

        uint64_t value = 0;
        for (const char& c : sv) {
            if (c < '0' || c > '9') { return std::nullopt; }
            value = (value * 10) + (c - '0');
        }

        return value;
    };

    const std::optional<uint64_t> ip_nums[] = {validate_digit(first),
                                               validate_digit(second),
                                               validate_digit(third),
                                               validate_digit(fourth)};
    if (std::ranges::any_of(ip_nums, [](const auto& opt) { return !opt || opt.value() >= 256; })) {
        return false;
    }

    const auto checked_port = validate_digit(port).value_or(SIZE_MAX);
    return checked_port >= 1 && checked_port <= 65'535;
}

bool ipv4_t::any_empty() const {
    return first.empty() || second.empty() || third.empty() || fourth.empty() || port.empty();
}

std::string ipv4_t::to_string() const {
    return fmt::format("{}.{}.{}.{}:{}", first, second, third, fourth, port);
}

} // namespace mbr
