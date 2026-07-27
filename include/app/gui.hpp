#pragma once
#include <memory>
#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <qtmetamacros.h>
#include "app/context.hpp"
#include "app/pages/page.hpp"
#include <stdx/assert.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

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
    std::shared_ptr<app_context> context_;
    ankerl::unordered_dense::map<page_type_t, QWidget*> page_lookup_;
};

} // namespace mbr
