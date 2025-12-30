#pragma once

static constexpr float DEFAULT_FONT_SIZE = 18.0f;

struct ImFont;

struct AppFonts {
    ImFont* Regular;
    ImFont* Bold;
    ImFont* Italic;
    ImFont* BoldItalic;
};

AppFonts LoadFonts();
void     SetDarkThemeColors();
