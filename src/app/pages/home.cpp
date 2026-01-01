#include <imgui.h>

#include "core/log.hpp"

#include "app/pages/home.hpp"

void HomePage::OnEnter() { LOG_INFO("Entered HomePage"); }
void HomePage::OnExit() { LOG_INFO("Exited HomePage"); }

void HomePage::Update() {
    const auto window_flags = DefaultWindowFlags();
    if (ImGui::Begin("Home Page", nullptr, window_flags)) {}

    ImGui::End();
}
