#pragma once
#include <memory>
#include <QMainWindow>
#include <QStackedWidget>
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

private:
    QStackedWidget*              pages_;
    pages::page*                 current_page_ = nullptr; // now owned by pages_, not unique_ptr
    std::shared_ptr<app_context> context_;
};

} // namespace mbr
