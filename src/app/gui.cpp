#include "app/gui.hpp"

#include <memory>
#include <string>

#include <fmt/format.h>
#include <gsl/pointers>
#include <gsl/util>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "app/assets/images/app_icon.hpp"
#include "app/assets/texture.hpp"
#include "app/context.hpp"
#include "app/pages/home.hpp"
#include "app/pages/rpm.hpp"
#include "app/pages/serialmon.hpp"
#include "app/pages/shock.hpp"
#include "app/pages/utils.hpp"
#include "app/pages/view.hpp"
#include "app/style.hpp"
#include "core/time.hpp"

using namespace std::chrono;

namespace mbr {

    gui_t::gui_t() {
        for (auto type : {page_type_t::HOME, page_type_t::PLOT, page_type_t::ANALYSIS, page_type_t::SERIAL}) {
            pages::page* p = create_page(type, context_, pages_);
            pages_->addWidget(p);
            page_lookup_[type] = p;
        }
    }

namespace {
    [[nodiscard]] pages::page* create_page(page_type_t type, const std::shared_ptr<app_context>& ctx, QWidget* parent) {
        switch (type) {
        case page_type_t::HOME:   return new pages::home_page(ctx, parent);
        case page_type_t::PLOT:    return new pages::rpm_page(ctx, parent);
        case page_type_t::ANALYSIS:  return new pages::shock_page(ctx, parent);
        case page_type_t::SERIAL: return new pages::serial_page(ctx, parent);
        default:                  return nullptr;
        }
    }

} // namespace

    void gui_t::change_page(page_type_t type) {
        pages_->setCurrentWidget(type);
    }


} // namespace mbr
