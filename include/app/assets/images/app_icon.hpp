#pragma once

#include <array>

namespace mbr::assets {

constexpr auto BAJA_LOGO_PNG = std::to_array<unsigned char>({
#include "app/assets/images/BajaLogo.png.inc"
});

} // namespace mbr::assets
