#include <array>
#include <cassert>
#include <optional>

#ifdef _WIN32
#    include <dwmapi.h>
#endif

#include <imgui.h>
#include <sokol_app.h>
#include <stdx/assert.hh>

#include "app/assets/fonts/open_sans.hpp"
#include "app/style.hpp"
#include "core/log.hpp"

namespace mbr {

namespace {

constinit std::optional<std::array<ImVec4, ImGuiCol_COUNT>> color_cache = std::nullopt;

void initialize_imgui_color_cache(ImVec4 colors[ImGuiCol_COUNT]) {
    auto& cache = color_cache.emplace();
    for (auto i = 0; i < ImGuiCol_COUNT; i++) { cache[i] = colors[i]; }
}

void refresh_imgui_color_cache(ImVec4 colors[ImGuiCol_COUNT]) {
    ASSERT(color_cache, "Color cache has not yet been initialized");
    auto& cache = *color_cache;
    for (auto i = 0; i < ImGuiCol_COUNT; i++) { colors[i] = cache[i]; }
}

} // namespace

app_fonts load_fonts() {
    ImGuiIO&     io = ImGui::GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;

    auto* regular =
        io.Fonts->AddFontFromMemoryTTF(assets::OPEN_SANS_REGULAR_TTF.data(),
                                       static_cast<int>(assets::OPEN_SANS_REGULAR_TTF.size()),
                                       DEFAULT_FONT_SIZE,
                                       &cfg);

    auto* bold = io.Fonts->AddFontFromMemoryTTF(assets::OPEN_SANS_BOLD_TTF.data(),
                                                static_cast<int>(assets::OPEN_SANS_BOLD_TTF.size()),
                                                DEFAULT_FONT_SIZE,
                                                &cfg);
    auto* italic =
        io.Fonts->AddFontFromMemoryTTF(assets::OPEN_SANS_ITALIC_TTF.data(),
                                       static_cast<int>(assets::OPEN_SANS_ITALIC_TTF.size()),
                                       DEFAULT_FONT_SIZE,
                                       &cfg);
    auto* bold_italic =
        io.Fonts->AddFontFromMemoryTTF(assets::OPEN_SANS_BOLD_ITALIC_TTF.data(),
                                       static_cast<int>(assets::OPEN_SANS_BOLD_ITALIC_TTF.size()),
                                       DEFAULT_FONT_SIZE,
                                       &cfg);

    return {.regular = regular, .bold = bold, .italic = italic, .bold_italic = bold_italic};
}

void app_style::set_dark_theme(const log_fn_t& log) {
    log_info(log, "Setting dark mode");
#ifdef _WIN32
    HWND hwnd          = reinterpret_cast<HWND>(const_cast<void*>(sapp_win32_get_hwnd()));
    BOOL use_dark_mode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    // This is from my Game Engine
    auto& colors = ImGui::GetStyle().Colors;
    if (!color_cache) { initialize_imgui_color_cache(colors); }
    refresh_imgui_color_cache(colors);

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

    dark_mode = true;
}

void app_style::set_light_theme(const log_fn_t& log) {
    log_info(log, "Setting light mode");
#ifdef _WIN32
    HWND hwnd          = reinterpret_cast<HWND>(const_cast<void*>(sapp_win32_get_hwnd()));
    BOOL use_dark_mode = FALSE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    // This isn't from my Game Engine
    auto& colors = ImGui::GetStyle().Colors;
    if (!color_cache) { initialize_imgui_color_cache(colors); }
    refresh_imgui_color_cache(colors);

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

    dark_mode = false;
}

} // namespace mbr
