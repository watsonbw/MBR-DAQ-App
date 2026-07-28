#include "app/pages/home.hpp"

#include <string>
#include <utility>

#include <fmt/format.h>
#include <gsl/util>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "core/log.hpp"
#include "core/time.hpp"

namespace mbr::pages {

void home_page::on_enter() { log_info(context_->log, "Entered HomePage"); }
void home_page::on_exit() { log_info(context_->log, "Exited HomePage"); }

void home_page::build_page() {
    PROFILE_FUNCTION();
    build_rhs();
    build_lhs();
}

void home_page::build_rhs() {

}

void home_page::build_lhs() {

}
} // namespace mbr::pages
