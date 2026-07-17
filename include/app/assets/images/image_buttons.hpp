#pragma once

#include <array>

namespace mbr {

constexpr auto PLAY_BUTTON_PNG = std::to_array<unsigned char>({
#include "app/assets/images/PlayButton.png.inc"
});

constexpr auto PAUSE_BUTTON_PNG = std::to_array<unsigned char>({
#include "app/assets/images/PauseButton.png.inc"
});

constexpr auto STEP_BUTTON_PNG = std::to_array<unsigned char>({
#include "app/assets/images/StepButton.png.inc"
});

} // namespace mbr
