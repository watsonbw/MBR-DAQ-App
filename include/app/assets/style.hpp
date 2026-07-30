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
inline constexpr QColor bg_dark(0x1e, 0x1e, 0x1e);
inline constexpr QColor bg_hover(0x2a, 0x2a, 0x2a);
inline constexpr QColor border(0x2d, 0x2d, 0x2d);
inline constexpr QColor text_main(0xd4, 0xd4, 0xd4);
inline constexpr QColor text_muted(0xa0, 0xa0, 0xa0);
inline constexpr QColor accent(0x00, 0xad, 0xb5);

// Status Indicator Colors
inline constexpr QColor status_error(0xf4, 0x43, 0x36);
inline constexpr QColor status_warn(0xff, 0xc1, 07);
inline constexpr QColor status_ok(0x00, 0xe6, 0x76);

// Graph Line Palette
inline constexpr std::array<QColor, 6> graph_palette = {QColor(0x00, 0xad, 0xb5),
                                                        QColor(0xff, 0x57, 0x22),
                                                        QColor(0xe9, 0x1e, 0x63),
                                                        QColor(0x9c, 0x27, 0xb0),
                                                        QColor(0x4c, 0xaf, 0x50),
                                                        QColor(0xff, 0xeb, 0x3b)};
QColor                                 get_graph_color(std::size_t index);

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
