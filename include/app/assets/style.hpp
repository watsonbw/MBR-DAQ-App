#pragma once

#include <QColor>
#include <QString>
#include <array>
#include <cstddef>

#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace mbr::ui::style {

namespace color {

// Dark Mode Theme Standards
inline constexpr QRgb bg_dark{0xff1e1e1e};
inline constexpr QRgb bg_hover{0xff2a2a2a};
inline constexpr QRgb border{0xff2d2d2d};
inline constexpr QRgb text_main{0xffd4d4d4};
inline constexpr QRgb text_muted{0xffa0a0a0};
inline constexpr QRgb accent{0xff00adb5};

// Status Indicator Colors
inline constexpr QRgb status_error{0xfff44336};
inline constexpr QRgb status_warn{0xffffc107};
inline constexpr QRgb status_ok{0xff00e676};

// Graph Line Palette
inline constexpr std::array<QRgb, 6> graph_palette = {
    0xff00adb5, 0xffff5722, 0xffe91e63, 0xff9c27b0, 0xff4caf50, 0xffffeb3b};
QColor get_graph_color(std::size_t index);

} // namespace color

// QSS Generators
struct button_style_options {
    int     font_size     = 10;
    int     padding_v     = 6;
    int     padding_h     = 12;
    int     border_radius = 4;
    QString align         = "center";
};

QString make_button_style(const button_style_options& opts = {});
QString make_tree_style();
QString make_menubar_style(int font_size_px = 20);
QString make_menu_style(int font_size_px = 20);
QString make_status_dot_style(const QColor& color, int diameter_px = 20);

} // namespace mbr::ui::style
