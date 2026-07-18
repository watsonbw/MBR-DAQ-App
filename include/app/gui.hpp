#pragma once

#include <memory>

#include "app/context.hpp"
#include "app/pages/page.hpp"
#include "assets/texture.hpp"

struct sapp_event;
struct sapp_desc;

namespace mbr {

class gui_t {
  public:
    explicit gui_t(const std::shared_ptr<app_context>& ctx) : context_{ctx} {};

    sapp_desc get_sokol_desc();
    void      on_init();
    void      on_frame();
    void      on_event(const sapp_event* event);
    void      on_cleanup();

  private:
    void change_page(page_type_t type);
    void draw_main_menu_bar();

  private:
    std::unique_ptr<pages::page_base> current_page_;
    std::shared_ptr<app_context>      context_;
    std::string                       command_buf_;
};

} // namespace mbr
