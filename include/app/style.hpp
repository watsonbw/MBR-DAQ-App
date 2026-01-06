#pragma once

struct ImFont;

struct AppFonts {
    ImFont* Regular;
    ImFont* Bold;
    ImFont* Italic;
    ImFont* BoldItalic;
};

AppFonts LoadFonts();

struct AppStyle {
    void SetDarkThemeColors();
    void SetLightThemeColors();

    AppFonts DefaultFonts;
    bool     DarkMode{true};
};

extern float default_font_size;
extern float header_font_size;
extern float main_menu_bar_font_size;
extern float main_menu_item_font_size;
extern float menu_bar_font_size;
extern float menu_item_font_size;

#define WITH_FONT(font, size, code)  \
    do {                             \
        ImGui::PushFont(font, size); \
        code;                        \
        ImGui::PopFont();            \
    } while (0)

#define BOLD_DEFAULT(B) WITH_FONT(m_Context->Style.DefaultFonts.Bold, default_font_size, B)
#define HEADER(B) WITH_FONT(m_Context->Style.DefaultFonts.Regular, header_font_size, B)
#define BOLD_HEADER(B) WITH_FONT(m_Context->Style.DefaultFonts.Bold, header_font_size, B)
#define MAIN_MENU_BAR(B) \
    WITH_FONT(m_Context->Style.DefaultFonts.Regular, main_menu_bar_font_size, B)
#define MAIN_MENU_BAR_ITEM(B) \
    WITH_FONT(m_Context->Style.DefaultFonts.Regular, main_menu_item_font_size, B)
