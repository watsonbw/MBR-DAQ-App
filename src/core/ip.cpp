#include "core/ip.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <system_error>

#include <fmt/format.h>
#include <stdx/option.hh>

namespace mbr {

namespace {

stdx::option<u64> validate_digit(std::string_view sv) noexcept {
    u64        value;
    const auto res = std::from_chars(sv.begin(), sv.end(), value);
    if (res.ec != std::errc{} || res.ptr != sv.end()) { return stdx::none; }
    return value;
};

} // namespace

bool ipv4_t::is_valid() const {
    const std::array ip_nums = {validate_digit(first),
                                validate_digit(second),
                                validate_digit(third),
                                validate_digit(fourth)};
    if (std::ranges::any_of(ip_nums, [](const auto& opt) { return !opt || *opt >= 256; })) {
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
