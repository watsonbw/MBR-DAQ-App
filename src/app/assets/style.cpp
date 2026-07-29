#include <fmt/format.h>

#include <stdx/types.hh>
#include <stdx/utility.hh>

#include <QString>
#include <QColor>

#include "app/assets/style.hpp"

namespace mbr::ui::style {

QColor color::get_graph_color(usize index) {
    return QColor(graph_palette[index % graph_palette.size()]);
}

QString make_button_style(const button_style_options& opts) {
    return QString::fromStdString(fmt::format(
        "QToolButton {{"
        "   background-color: {0}; color: {1}; border: 1px solid {2};"
        "   border-radius: {3}px; padding: {4}px {5}px; font: {6}pt; text-align: {7};"
        "}}"
        "QToolButton:hover {{ background-color: {8}; }}",
        color::bg_dark.name().toStdString(),
        color::text_main.name().toStdString(),
        color::border.name().toStdString(),
        opts.border_radius,
        opts.padding_v,
        opts.padding_h,
        opts.font_size,
        opts.align.toStdString(),
        color::bg_hover.name().toStdString()
    ));
}

QString make_tree_style() {
    return QString::fromStdString(fmt::format(
        "QTreeWidget {{"
        "   background-color: {0};"
        "   color: {1};"
        "   border: 1px solid {2};"
        "   font: 14px;"
        "}}",
        color::bg_dark.name().toStdString(),
        color::text_main.name().toStdString(),
        color::border.name().toStdString(),
        color::accent.name().toStdString(),
        color::text_main.name().toStdString()
    ));
}

QString make_menubar_style(int font_size_px) {
    return QString::fromStdString(fmt::format(
        "QMenuBar {{"
        "   background-color: {0}; color: {1}; font: {2}px; spacing: 2px;"
        "   border-bottom: 1px solid {3};"
        "}}"
        "QMenuBar::item {{"
        "   background-color: transparent; padding: 4px 8px; border-radius: 4px; border: 1px solid transparent;"
        "}}"
        "QMenuBar::item:selected {{"
        "   background-color: {4}; border: 1px solid {3};"
        "}}",
        color::bg_dark.name().toStdString(),
        color::text_main.name().toStdString(),
        font_size_px,
        color::border.name().toStdString(),
        color::bg_hover.name().toStdString()
    ));
}

QString make_menu_style(int font_size_px) {
    return QString::fromStdString(fmt::format(
        "QMenu {{"
        "   background-color: {0}; color: {1}; font-size: {2}px; border: 1px solid {3}; padding: 4px;"
        "}}"
        "QMenu::item {{"
        "   background-color: transparent; padding: 4px 24px 4px 12px; border-radius: 4px; border: 1px solid transparent;"
        "}}"
        "QMenu::item:selected {{"
        "   background-color: {4}; border: 1px solid {3};"
        "}}"
        "QMenu::item:disabled {{ color: {5}; }}"
        "QMenu::separator {{ height: 1px; background-color: {3}; margin: 4px 6px; }}",
        color::bg_dark.name().toStdString(),
        color::text_main.name().toStdString(),
        font_size_px,
        color::border.name().toStdString(),
        color::bg_hover.name().toStdString(),
        color::text_muted.name().toStdString()
    ));
}

QString make_status_dot_style(const QColor& color, int diameter_px) {
    return QString::fromStdString(fmt::format(
        "background-color: {}; border-radius: {}px;",
        color.name().toStdString(),
        diameter_px / 2
    ));
}

} // namespace mbr::ui::style
