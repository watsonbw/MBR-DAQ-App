#pragma once

#include <QColor>
#include <QString>

#include "colors.hpp"

#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace mbr::ui::style {


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
