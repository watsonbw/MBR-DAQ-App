#include "app/context.hpp"
#include "app/style.hpp"

#include "app/pages/page.hpp"

#include <imgui.h>

const char* PageTypeString(PageType type) {
    switch (type) {
    case PageType::HOME:
        return "Home";
    case PageType::RPM:
        return "RPM";
    case PageType::SHOCK:
        return "Shock";
    case PageType::VIEW:
        return "View";
    default:
        return "Unknown";
    }
}

int DefaultWindowFlags() {
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoNavFocus;
    return window_flags;
}
