#include "app/context.hpp"

namespace mbr {

const char* PageTypeString(PageType type) {
    switch (type) {
    case PageType::HOME:   return "Home";
    case PageType::RPM:    return "RPM";
    case PageType::SHOCK:  return "Shock";
    case PageType::VIEW:   return "View";
    case PageType::SERIAL: return "Serial Monitor";
    default:               return "Unknown";
    }
}

} // namespace mbr
