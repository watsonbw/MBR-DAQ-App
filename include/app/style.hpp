#pragma once

struct ImFont;

namespace mbr {

struct app_fonts {
    ImFont* regular;
    ImFont* bold;
    ImFont* italic;
    ImFont* bold_italic;
};

app_fonts load_fonts();

struct app_style {
    void set_dark_theme();
    void set_light_theme();

    app_fonts default_fonts;
    bool      dark_mode{true};
};

constexpr float DEFAULT_FONT_SIZE        = 22.0F;
constexpr float HEADER_FONT_SIZE         = 26.0F;
constexpr float MAIN_MENU_BAR_FONT_SIZE  = 30.0F;
constexpr float MAIN_MENU_ITEM_FONT_SIZE = 28.0F;
constexpr float MENU_BAR_FONT_SIZE       = 26.0F;
constexpr float MENU_ITEM_FONT_SIZE      = 24.0F;

#define WITH_FONT(font, size, code)  \
    do {                             \
        ImGui::PushFont(font, size); \
        code;                        \
        ImGui::PopFont();            \
    } while (false)

#define BOLD_DEFAULT(B) WITH_FONT(context_->style.default_fonts.bold, DEFAULT_FONT_SIZE, B)
#define HEADER(B) WITH_FONT(context_->style.default_fonts.regular, HEADER_FONT_SIZE, B)
#define BOLD_HEADER(B) WITH_FONT(context_->style.default_fonts.bold, HEADER_FONT_SIZE, B)
#define MAIN_MENU_BAR(B) \
    WITH_FONT(context_->style.default_fonts.regular, MAIN_MENU_BAR_FONT_SIZE, B)
#define MAIN_MENU_BAR_ITEM(B) \
    WITH_FONT(context_->style.default_fonts.regular, MAIN_MENU_ITEM_FONT_SIZE, B)

} // namespace mbr
