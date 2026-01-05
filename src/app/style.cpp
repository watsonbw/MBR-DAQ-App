#ifdef _WIN32
#include <dwmapi.h>
#endif

#include <sokol_app.h>

#include <imgui.h>

#include "core/log.hpp"

#include "app/assets/fonts/OpenSans.hpp"
#include "app/style.hpp"

AppFonts LoadFonts() {
    ImGuiIO&     io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;

    auto* regular = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansRegular_ttf),
                                                   OpenSansRegular_ttf_size,
                                                   DEFAULT_FONT_SIZE,
                                                   &cfg);

    auto* bold   = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansBold_ttf),
                                                OpenSansBold_ttf_size,
                                                DEFAULT_FONT_SIZE,
                                                &cfg);
    auto* italic = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansItalic_ttf),
                                                  OpenSansItalic_ttf_size,
                                                  DEFAULT_FONT_SIZE,
                                                  &cfg);
    auto* bold_italic =
        io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansBoldItalic_ttf),
                                       OpenSansBoldItalic_ttf_size,
                                       DEFAULT_FONT_SIZE,
                                       &cfg);

    return {.Regular = regular, .Bold = bold, .Italic = italic, .BoldItalic = bold_italic};
}

void AppStyle::SetDarkThemeColors() {
    LOG_INFO("Setting dark mode");
#ifdef _WIN32
    HWND hwnd          = (HWND)sapp_win32_get_hwnd();
    BOOL use_dark_mode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    // This is from my Game Engine
    auto& colors              = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4{0.1f, 0.105f, 0.11f, 1.0f};

    // Headers
    colors[ImGuiCol_Header]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_HeaderActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_ButtonActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

    // Frame BG
    colors[ImGuiCol_FrameBg]        = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_FrameBgActive]  = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab]                = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TabHovered]         = ImVec4{0.38f, 0.3805f, 0.381f, 1.0f};
    colors[ImGuiCol_TabActive]          = ImVec4{0.28f, 0.2805f, 0.281f, 1.0f};
    colors[ImGuiCol_TabUnfocused]       = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2f, 0.205f, 0.21f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg]          = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TitleBgActive]    = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15f, 0.1505f, 0.151f, 1.0f};

    DarkMode = true;
}

void AppStyle::SetLightThemeColors() {
    LOG_INFO("Setting light mode");
#ifdef _WIN32
    HWND hwnd          = (HWND)sapp_win32_get_hwnd();
    BOOL use_dark_mode = FALSE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    DarkMode = false;
}
