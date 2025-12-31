#include <iostream>

#include <imgui.h>

#include "app/pages/home.hpp"

void HomePage::OnEnter() { std::cout << "[INFO]: Entered HomePage" << "\n"; }
void HomePage::OnExit() { std::cout << "[INFO]: Exitted HomePage" << "\n"; }

void HomePage::Update() {
    const auto window_flags = DefaultWindowFlags();
    if (ImGui::Begin("Home Page", nullptr, window_flags)) {}

    ImGui::End();
}
