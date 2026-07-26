#pragma once
#include <memory>
#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <qtmetamacros.h>
#include "app/context.hpp"
#include "app/pages/page.hpp"

namespace mbr {

class gui_t : public QMainWindow {
    Q_OBJECT
public:
    explicit gui_t(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr);

private:
    void change_page(page_type_t type);
    void build_menu_bar();
    [[nodiscard]] pages::page* create_page(page_type_t type, const std::shared_ptr<app_context>& ctx, QWidget* parent);

private:
    QStackedWidget*              pages_;
    pages::page*                 current_page_ = nullptr; // now owned by pages_, not unique_ptr
    //homepage*                  home_page_;
    //plot_page*                 plot_page;
    //analysis_page*             analysis_page_;
    //serial_page*               serial_page;
    std::shared_ptr<app_context> context_;
};

} // namespace mbr
