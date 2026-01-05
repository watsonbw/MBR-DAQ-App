#pragma once

#include <array>

static constexpr float DEFAULT_FONT_SIZE = 18.0f;

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
