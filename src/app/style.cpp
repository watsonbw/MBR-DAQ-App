#include <array>
#include <cassert>
#include <optional>

#ifdef _WIN32
#include <dwmapi.h>
#endif

#include <sokol_app.h>

#include <imgui.h>

#include "core/log.hpp"

#include "app/assets/fonts/OpenSans.hpp"
#include "app/style.hpp"

static std::optional<std::array<ImVec4, ImGuiCol_COUNT>> color_cache = std::nullopt;

AppFonts LoadFonts() {
    ImGuiIO&     io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;

    auto* regular =
        io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OPEN_SANS_REGULAR_TTF),
                                       static_cast<int>(OPEN_SANS_REGULAR_TTF_SIZE),
                                       default_font_size,
                                       &cfg);

    auto* bold   = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OPEN_SANS_BOLD_TTF),
                                                static_cast<int>(OPEN_SANS_BOLD_TTF_SIZE),
                                                default_font_size,
                                                &cfg);
    auto* italic = io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OPEN_SANS_ITALIC_TTF),
                                                  static_cast<int>(OPEN_SANS_ITALIC_TTF_SIZE),
                                                  default_font_size,
                                                  &cfg);
    auto* bold_italic =
        io.Fonts->AddFontFromMemoryTTF(const_cast<unsigned char*>(OPEN_SANS_BOLD_ITALIC_TTF),
                                       static_cast<int>(OPEN_SANS_BOLD_ITALIC_TTF_SIZE),
                                       default_font_size,
                                       &cfg);

    return {.Regular = regular, .Bold = bold, .Italic = italic, .BoldItalic = bold_italic};
}

static void PopulateImGuiColorCache(ImVec4 colors[ImGuiCol_COUNT]) {
    color_cache = std::array<ImVec4, ImGuiCol_COUNT>{};
    for (auto i = 0; i < ImGuiCol_COUNT; i++) {
        color_cache.value()[i] = colors[i];
    }
}

static void RefreshImGuiColorCache(ImVec4 colors[ImGuiCol_COUNT]) {
    assert(color_cache);
    for (auto i = 0; i < ImGuiCol_COUNT; i++) {
        colors[i] = color_cache.value()[i];
    }
}

void AppStyle::SetDarkThemeColors() {
    LOG_INFO("Setting dark mode");
#ifdef _WIN32
    HWND hwnd          = reinterpret_cast<HWND>(const_cast<void*>(sapp_win32_get_hwnd()));
    BOOL use_dark_mode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    // This is from my Game Engine
    auto& colors = ImGui::GetStyle().Colors;
    if (!color_cache) {
        PopulateImGuiColorCache(colors);
        assert(color_cache);
    }
    RefreshImGuiColorCache(colors);

    colors[ImGuiCol_WindowBg] = {0.1F, 0.105F, 0.11F, 1.0F};

    // Headers
    colors[ImGuiCol_Header]        = {0.2F, 0.205F, 0.21F, 1.0F};
    colors[ImGuiCol_HeaderHovered] = {0.3F, 0.305F, 0.31F, 1.0F};
    colors[ImGuiCol_HeaderActive]  = {0.15F, 0.1505F, 0.151F, 1.0F};

    // Buttons
    colors[ImGuiCol_Button]        = {0.2F, 0.205F, 0.21F, 1.0F};
    colors[ImGuiCol_ButtonHovered] = {0.3F, 0.305F, 0.31F, 1.0F};
    colors[ImGuiCol_ButtonActive]  = {0.15F, 0.1505F, 0.151F, 1.0F};

    // Frame BG
    colors[ImGuiCol_FrameBg]        = {0.2F, 0.205F, 0.21F, 1.0F};
    colors[ImGuiCol_FrameBgHovered] = {0.3F, 0.305F, 0.31F, 1.0F};
    colors[ImGuiCol_FrameBgActive]  = {0.15F, 0.1505F, 0.151F, 1.0F};

    // Tabs
    colors[ImGuiCol_Tab]                = {0.15F, 0.1505F, 0.151F, 1.0F};
    colors[ImGuiCol_TabHovered]         = {0.38F, 0.3805F, 0.381F, 1.0F};
    colors[ImGuiCol_TabActive]          = {0.28F, 0.2805F, 0.281F, 1.0F};
    colors[ImGuiCol_TabUnfocused]       = {0.15F, 0.1505F, 0.151F, 1.0F};
    colors[ImGuiCol_TabUnfocusedActive] = {0.2F, 0.205F, 0.21F, 1.0F};

    // Title
    colors[ImGuiCol_TitleBg]          = {0.15F, 0.1505F, 0.151F, 1.0F};
    colors[ImGuiCol_TitleBgActive]    = {0.15F, 0.1505F, 0.151F, 1.0F};
    colors[ImGuiCol_TitleBgCollapsed] = {0.15F, 0.1505F, 0.151F, 1.0F};

    DarkMode = true;
}

void AppStyle::SetLightThemeColors() {
    LOG_INFO("Setting light mode");
#ifdef _WIN32
    HWND hwnd          = reinterpret_cast<HWND>(const_cast<void*>(sapp_win32_get_hwnd()));
    BOOL use_dark_mode = FALSE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    // This isn't from my Game Engine
    auto& colors = ImGui::GetStyle().Colors;
    if (!color_cache) {
        PopulateImGuiColorCache(colors);
        assert(color_cache);
    }
    RefreshImGuiColorCache(colors);

    colors[ImGuiCol_WindowBg] = {0.94F, 0.94F, 0.94F, 1.0F};
    colors[ImGuiCol_ChildBg]  = {0.00F, 0.00F, 0.00F, 0.00F};
    colors[ImGuiCol_PopupBg]  = {1.00F, 1.00F, 1.00F, 0.98F};

    // Text
    colors[ImGuiCol_Text]            = {0.15F, 0.15F, 0.15F, 1.0F};
    colors[ImGuiCol_TextDisabled]    = {0.50F, 0.50F, 0.50F, 1.00F};
    colors[ImGuiCol_InputTextCursor] = {0.15F, 0.15F, 0.15F, 1.0F};

    // Headers
    colors[ImGuiCol_Header]        = {0.85F, 0.85F, 0.85F, 1.0F};
    colors[ImGuiCol_HeaderHovered] = {0.78F, 0.78F, 0.78F, 1.0F};
    colors[ImGuiCol_HeaderActive]  = {0.70F, 0.70F, 0.70F, 1.0F};

    // Buttons
    colors[ImGuiCol_Button]        = {0.85F, 0.85F, 0.85F, 1.0F};
    colors[ImGuiCol_ButtonHovered] = {0.75F, 0.75F, 0.75F, 1.0F};
    colors[ImGuiCol_ButtonActive]  = {0.65F, 0.65F, 0.65F, 1.0F};

    // Frame BG
    colors[ImGuiCol_FrameBg]        = {1.00F, 1.00F, 1.00F, 1.0F};
    colors[ImGuiCol_FrameBgHovered] = {0.90F, 0.90F, 0.95F, 1.0F};
    colors[ImGuiCol_FrameBgActive]  = {0.85F, 0.85F, 0.90F, 1.0F};

    // Tabs
    colors[ImGuiCol_Tab]                = {0.85F, 0.85F, 0.85F, 1.0F};
    colors[ImGuiCol_TabHovered]         = {0.95F, 0.95F, 0.95F, 1.0F};
    colors[ImGuiCol_TabActive]          = {0.94F, 0.94F, 0.94F, 1.0F};
    colors[ImGuiCol_TabUnfocused]       = {0.80F, 0.80F, 0.80F, 1.0F};
    colors[ImGuiCol_TabUnfocusedActive] = {0.88F, 0.88F, 0.88F, 1.0F};

    // Title
    colors[ImGuiCol_TitleBg]          = {0.90F, 0.90F, 0.90F, 1.0F};
    colors[ImGuiCol_TitleBgActive]    = {0.90F, 0.90F, 0.90F, 1.0F};
    colors[ImGuiCol_TitleBgCollapsed] = {0.90F, 0.90F, 0.90F, 1.0F};
    colors[ImGuiCol_MenuBarBg]        = {0.90F, 0.90F, 0.90F, 1.0F};

    // Separators & Borders
    colors[ImGuiCol_Separator] = {0.70F, 0.70F, 0.70F, 1.0F};
    colors[ImGuiCol_Border]    = {0.70F, 0.70F, 0.70F, 0.5F};

    DarkMode = false;
}

float default_font_size        = 22.0F;
float header_font_size         = 26.0F;
float main_menu_bar_font_size  = 30.0F;
float main_menu_item_font_size = 28.0F;
float menu_bar_font_size       = 26.0F;
float menu_item_font_size      = 24.0F;
