#include "app/pages/home.hpp"

#include <imgui.h>

void HomePage::Update() {
    const auto window_flags = DefaultWindowFlags();
    if (ImGui::Begin("Home Page", nullptr, window_flags)) {}

    ImGui::End();
}
