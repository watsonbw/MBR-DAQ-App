#include <array>

#ifdef _WIN32
#include <dwmapi.h>
#endif

#include <sokol_app.h>

#include <imgui.h>

#include "core/log.hpp"

#include "app/assets/fonts/OpenSans.hpp"
#include "app/style.hpp"

static std::array<ImVec4, ImGuiCol_COUNT> ColorCache = {};

AppFonts LoadFonts() {
    ImGuiIO&     io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;

    auto* regular = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansRegular_ttf),
                                                   OpenSansRegular_ttf_size,
                                                   default_font_size,
                                                   &cfg);

    auto* bold   = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansBold_ttf),
                                                OpenSansBold_ttf_size,
                                                default_font_size,
                                                &cfg);
    auto* italic = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansItalic_ttf),
                                                  OpenSansItalic_ttf_size,
                                                  default_font_size,
                                                  &cfg);
    auto* bold_italic =
        io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OpenSansBoldItalic_ttf),
                                       OpenSansBoldItalic_ttf_size,
                                       default_font_size,
                                       &cfg);

    return {.Regular = regular, .Bold = bold, .Italic = italic, .BoldItalic = bold_italic};
}

static void PopulateImGuiColorCache(ImVec4 colors[ImGuiCol_COUNT]) {
    for (auto i = 0; i < ImGuiCol_COUNT; i++) {
        ColorCache[i] = colors[i];
    }
}

static void RefreshImGuiColorCache(ImVec4 colors[ImGuiCol_COUNT]) {
    for (auto i = 0; i < ImGuiCol_COUNT; i++) {
        colors[i] = ColorCache[i];
    }
}

void AppStyle::SetDarkThemeColors() {
    LOG_INFO("Setting dark mode");
#ifdef _WIN32
    HWND hwnd          = (HWND)sapp_win32_get_hwnd();
    BOOL use_dark_mode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    // This is from my Game Engine
    auto& colors = ImGui::GetStyle().Colors;
    if (!m_CachedInital) {
        PopulateImGuiColorCache(colors);
        m_CachedInital = true;
    }
    RefreshImGuiColorCache(colors);

    colors[ImGuiCol_WindowBg] = {0.1f, 0.105f, 0.11f, 1.0f};

    // Headers
    colors[ImGuiCol_Header]        = {0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = {0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_HeaderActive]  = {0.15f, 0.1505f, 0.151f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button]        = {0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = {0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_ButtonActive]  = {0.15f, 0.1505f, 0.151f, 1.0f};

    // Frame BG
    colors[ImGuiCol_FrameBg]        = {0.2f, 0.205f, 0.21f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = {0.3f, 0.305f, 0.31f, 1.0f};
    colors[ImGuiCol_FrameBgActive]  = {0.15f, 0.1505f, 0.151f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab]                = {0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TabHovered]         = {0.38f, 0.3805f, 0.381f, 1.0f};
    colors[ImGuiCol_TabActive]          = {0.28f, 0.2805f, 0.281f, 1.0f};
    colors[ImGuiCol_TabUnfocused]       = {0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = {0.2f, 0.205f, 0.21f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg]          = {0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TitleBgActive]    = {0.15f, 0.1505f, 0.151f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = {0.15f, 0.1505f, 0.151f, 1.0f};

    DarkMode = true;
}

void AppStyle::SetLightThemeColors() {
    LOG_INFO("Setting light mode");
#ifdef _WIN32
    HWND hwnd          = (HWND)sapp_win32_get_hwnd();
    BOOL use_dark_mode = FALSE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    // This isn't from my Game Engine
    auto& colors = ImGui::GetStyle().Colors;
    if (!m_CachedInital) {
        PopulateImGuiColorCache(colors);
        m_CachedInital = true;
    }
    RefreshImGuiColorCache(colors);

    colors[ImGuiCol_WindowBg] = {0.94f, 0.94f, 0.94f, 1.0f};
    colors[ImGuiCol_ChildBg]  = {0.00f, 0.00f, 0.00f, 0.00f};
    colors[ImGuiCol_PopupBg]  = {1.00f, 1.00f, 1.00f, 0.98f};

    // Text
    colors[ImGuiCol_Text]         = {0.15f, 0.15f, 0.15f, 1.0f};
    colors[ImGuiCol_TextDisabled] = {0.50f, 0.50f, 0.50f, 1.00f};

    // Headers
    colors[ImGuiCol_Header]        = {0.85f, 0.85f, 0.85f, 1.0f};
    colors[ImGuiCol_HeaderHovered] = {0.78f, 0.78f, 0.78f, 1.0f};
    colors[ImGuiCol_HeaderActive]  = {0.70f, 0.70f, 0.70f, 1.0f};

    // Buttons
    colors[ImGuiCol_Button]        = {0.85f, 0.85f, 0.85f, 1.0f};
    colors[ImGuiCol_ButtonHovered] = {0.75f, 0.75f, 0.75f, 1.0f};
    colors[ImGuiCol_ButtonActive]  = {0.65f, 0.65f, 0.65f, 1.0f};

    // Frame BG
    colors[ImGuiCol_FrameBg]        = {1.00f, 1.00f, 1.00f, 1.0f};
    colors[ImGuiCol_FrameBgHovered] = {0.90f, 0.90f, 0.95f, 1.0f};
    colors[ImGuiCol_FrameBgActive]  = {0.85f, 0.85f, 0.90f, 1.0f};

    // Tabs
    colors[ImGuiCol_Tab]                = {0.85f, 0.85f, 0.85f, 1.0f};
    colors[ImGuiCol_TabHovered]         = {0.95f, 0.95f, 0.95f, 1.0f};
    colors[ImGuiCol_TabActive]          = {0.94f, 0.94f, 0.94f, 1.0f};
    colors[ImGuiCol_TabUnfocused]       = {0.80f, 0.80f, 0.80f, 1.0f};
    colors[ImGuiCol_TabUnfocusedActive] = {0.88f, 0.88f, 0.88f, 1.0f};

    // Title
    colors[ImGuiCol_TitleBg]          = {0.90f, 0.90f, 0.90f, 1.0f};
    colors[ImGuiCol_TitleBgActive]    = {0.90f, 0.90f, 0.90f, 1.0f};
    colors[ImGuiCol_TitleBgCollapsed] = {0.90f, 0.90f, 0.90f, 1.0f};
    colors[ImGuiCol_MenuBarBg]        = {0.90f, 0.90f, 0.90f, 1.0f};

    // Separators & Borders
    colors[ImGuiCol_Separator] = {0.70f, 0.70f, 0.70f, 1.0f};
    colors[ImGuiCol_Border]    = {0.70f, 0.70f, 0.70f, 0.5f};

    DarkMode = false;
}

float default_font_size        = 22.0f;
float header_font_size         = 26.0f;
float main_menu_bar_font_size  = 30.0f;
float main_menu_item_font_size = 28.0f;
float menu_bar_font_size       = 26.0f;
float menu_item_font_size      = 24.0f;
