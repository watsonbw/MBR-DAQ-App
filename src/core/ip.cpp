#include <algorithm>
#include <cstdint>
#include <format>
#include <optional>
#include <string_view>

#include "core/ip.hpp"

bool IpV4::Valid() const {
    const auto validate_digit = [](std::string_view sv) -> std::optional<uint64_t> {
        if (sv.empty()) { return std::nullopt; }

        uint64_t value = 0;
        for (char c : sv) {
            if (c < '0' || c > '9') { return std::nullopt; }
            value = value * 10 + (c - '0');
        }

        return value;
    };

    const std::optional<uint64_t> ip_nums[] = {validate_digit(First),
                                               validate_digit(Second),
                                               validate_digit(Third),
                                               validate_digit(Fourth)};
    if (std::ranges::any_of(ip_nums, [](const auto& opt) { return !opt || opt.value() >= 256; })) {
        return false;
    }

    const auto port = validate_digit(Port).value_or(SIZE_MAX);
    return port >= 1 && port <= 65535;
}

bool IpV4::AnyEmpty() const {
    return First.empty() || Second.empty() || Third.empty() || Fourth.empty() || Port.empty();
}

std::string IpV4::String() const {
    return std::format("{}.{}.{}.{}:{}", First, Second, Third, Fourth, Port);
}
