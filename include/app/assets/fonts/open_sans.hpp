#pragma once

#include <array>

namespace mbr::assets {

constinit inline auto OPEN_SANS_REGULAR_TTF = std::to_array<unsigned char>({
#include "app/assets/fonts/OpenSans-Regular.ttf.inc"
});

constinit inline auto OPEN_SANS_BOLD_TTF = std::to_array<unsigned char>({
#include "app/assets/fonts/OpenSans-Bold.ttf.inc"
});

constinit inline auto OPEN_SANS_ITALIC_TTF = std::to_array<unsigned char>({
#include "app/assets/fonts/OpenSans-Italic.ttf.inc"
});

constinit inline auto OPEN_SANS_BOLD_ITALIC_TTF = std::to_array<unsigned char>({
#include "app/assets/fonts/OpenSans-BoldItalic.ttf.inc"
});

} // namespace mbr::assets
