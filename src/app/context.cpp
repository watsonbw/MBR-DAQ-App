#include "app/context.hpp"

const char* PageTypeString(PageType type) {
    switch (type) {
    case PageType::HOME:
        return "Home";
    case PageType::RPM:
        return "RPM";
    case PageType::SHOCK:
        return "Shock";
    case PageType::VIEW:
        return "View";
    default:
        return "Unknown";
    }
}
