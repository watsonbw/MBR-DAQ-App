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
inline const QColor bg_dark{"#1e1e1e"};
inline const QColor bg_hover{"#2a2a2a"};
inline const QColor border{"#2d2d2d"};
inline const QColor text_main{"#d4d4d4"};
inline const QColor text_muted{"#a0a0a0"};
inline const QColor accent{"#00adb5"};

// Status Indicator Colors
inline const QColor status_error{"#f44336"};
inline const QColor status_warn{"#ffc107"};
inline const QColor status_ok{"#00e676"};

// Graph Line Palette
inline const std::array<QColor, 6> graph_palette = {
    "#00adb5", "#ff5722", "#e91e63", "#9c27b0", "#4caf50", "#ffeb3b"};

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
