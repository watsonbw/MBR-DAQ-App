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

  private:
    bool m_CachedInital;
};

static float DEFAULT_FONT_SIZE        = 22.0f;
static float HEADER_FONT_SIZE         = 26.0f;
static float MAIN_MENU_BAR_FONT_SIZE  = 30.0f;
static float MAIN_MENU_ITEM_FONT_SIZE = 28.0f;
static float MENU_BAR_FONT_SIZE       = 26.0f;
static float MENU_ITEM_FONT_SIZE      = 24.0f;

#define WITH_FONT(font, size, code)  \
    do {                             \
        ImGui::PushFont(font, size); \
        code;                        \
        ImGui::PopFont();            \
    } while (0)

#define BOLD_DEFAULT(B) WITH_FONT(m_Context->Style.DefaultFonts.Bold, DEFAULT_FONT_SIZE, B)
#define HEADER(B) WITH_FONT(m_Context->Style.DefaultFonts.Regular, HEADER_FONT_SIZE, B)
#define BOLD_HEADER(B) WITH_FONT(m_Context->Style.DefaultFonts.Bold, HEADER_FONT_SIZE, B)
#define MAIN_MENU_BAR(B) \
    WITH_FONT(m_Context->Style.DefaultFonts.Regular, MAIN_MENU_BAR_FONT_SIZE, B)
#define MAIN_MENU_BAR_ITEM(B) \
    WITH_FONT(m_Context->Style.DefaultFonts.Regular, MAIN_MENU_ITEM_FONT_SIZE, B)
