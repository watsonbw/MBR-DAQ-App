#pragma once

#define GL_WHITE 1.0, 1.0, 1.0, 1.0f

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
