#include "helpers/tempfile.hpp"

#include <atomic>
#include <filesystem>
#include <random>
#include <string_view>

#include <fmt/format.h>
#include <stdx/types.hh>

namespace mbr::tests::helpers {

namespace {

const u64        seed{std::random_device{}()};
std::atomic<u64> counter{0};

std::filesystem::path tempfile_path(std::string_view tag) {
    const auto dir{std::filesystem::temp_directory_path()};
    const auto name{fmt::format("cairn_{}_{}_{}", tag, seed, counter.fetch_add(1))};
    return dir / name;
}

} // namespace

tempfile::tempfile(std::string_view tag) : path{tempfile_path(tag)} {
    std::filesystem::remove(path);
}

tempfile::~tempfile() { std::filesystem::remove(path); }

} // namespace mbr::tests::helpers
