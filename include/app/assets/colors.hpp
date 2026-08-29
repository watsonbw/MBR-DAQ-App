#include <QColor>
#include <QPalette>

namespace mbr::ui::style {

namespace color {

// Dark Mode Theme Standards
inline constexpr QColor bg_d(0x1e, 0x1e, 0x1e);
inline constexpr QColor bg_hover_d(0x2a, 0x2a, 0x2a);
inline constexpr QColor border_d(0x2d, 0x2d, 0x2d);
inline constexpr QColor text_main_d(0xd4, 0xd4, 0xd4);
inline constexpr QColor text_muted_d(0xa0, 0xa0, 0xa0);
inline constexpr QColor accent_d(0x00, 0xad, 0xb5);


// Light Mode Theme Standards
inline constexpr QColor bg_l(0xf5, 0xf5, 0xf5);
inline constexpr QColor bg_hover_l(0xe8, 0xe8, 0xe8);
inline constexpr QColor border_l(0xd0, 0xd0, 0xd0);
inline constexpr QColor text_main_l(0x1e, 0x1e, 0x1e);
inline constexpr QColor text_muted_l(0x60, 0x60, 0x60);
inline constexpr QColor accent_l(0x00, 0x8a, 0x91);

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

} // namespace color

} // namespace mbr::ui::style
