#pragma once

#include <filesystem>
#include <string_view>

#include <stdx/utility.hh>

namespace mbr::tests::helpers {

struct tempfile {
    std::filesystem::path path;

    // Generates a path for a tempfile and ensures it does not exist on disk
    explicit tempfile(std::string_view tag);
    ~tempfile();
    MAKE_PINNED(tempfile);
};

} // namespace mbr::tests::helpers
